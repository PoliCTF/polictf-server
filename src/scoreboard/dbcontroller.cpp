#include "dbcontroller.hpp"
#include "cache_objects.hpp"

#include <sys/types.h>
#include <thread>
#include <memory>
#include <mutex>

#include <cppcms/json.h>
#include <cppcms/application.h>
#include <cppcms/http_context.h>
#include <cppcms/service.h>
#include <cppcms/http_request.h>
#include <cppcms/cache_interface.h>
#include <cppcms/serialization.h>
#include <cppcms/session_interface.h>
#include <booster/log.h>

#include <QtSql>
#include <mysql/errmsg.h>

extern cppcms::service *srv;
extern __thread cppcms::http::context *cur_ctx;

dbcontroller::dbcontroller() {
    cppcms::json::value settings = srv->settings();
    std::stringstream ss;
    ss.clear();
    ss.str("conn_thread_");
    ss << std::this_thread::get_id();
    this->db = QSqlDatabase::addDatabase("QMYSQL", QString::fromStdString(ss.str()));
    this->db.setHostName(QString::fromStdString(settings.get<std::string>("db.hostname")));
    this->db.setDatabaseName(QString::fromStdString(settings.get<std::string>("db.dbname")));
    this->db.setUserName(QString::fromStdString(settings.get<std::string>("db.username")));
    this->db.setPassword(QString::fromStdString(settings.get<std::string>("db.password")));

    if (!this->db.open()) {
        BOOSTER_CRITICAL("scoreboard") << "Error while connecting to database. Aborting.";
        throw 1;
    }

    BOOSTER_DEBUG("scoreboard") << "Opened db conn: " << this->db.connectionName().toStdString() << " from thread " << std::this_thread::get_id();
}

std::mutex dbcontroller::instances_mutex;
std::unordered_map<std::thread::id, std::shared_ptr<dbcontroller>> dbcontroller::thread_instances;

/**
 * @brief Handle a SQL error if possible
 * @param err error object to be handled
 * @return true if the error was successfully handled, false if not
 */
bool dbcontroller::handleError(QSqlError err) {
    switch (err.number()) {
    case CR_SERVER_GONE_ERROR:
    case CR_SERVER_LOST:
        db.close();
        db.open();
        BOOSTER_INFO("scoreboard") << "Reopening connection to database.";
        return true;

    case -1:
        BOOSTER_ERROR("scoreboard") << "Unknown DB error";
        return false;

    default:
        BOOSTER_ERROR("scoreboard") << "Error from database: " << err.number() << " - " << err.text().toStdString();
        return false;
    }
}

bool dbcontroller::doQuery(std::shared_ptr<QSqlQuery> stmt, std::function<bool(std::shared_ptr<QSqlQuery>)> prepare, int maxretries) {
    bool prepared = false;

    for (int retry = 0; retry < maxretries; ++retry) {
        if (prepare(stmt)) {
            prepared = true;
            break;
        }
        if (!handleError(stmt->lastError())) break; // Could not handle error, bail out
    }

    if (!prepared) {
        BOOSTER_ERROR("scoreboard") << "Could not prepare query for max retries, returning error";
        return false;
    }

    for (int retry = 0; retry < maxretries; ++retry) {
        if (stmt->exec()) return true; // Success!
        if (!handleError(stmt->lastError())) return false; // Could not handle error, bail out
    }

    BOOSTER_ERROR("scoreboard") << "Query failed for max retries, returning error";
    return false;
}

std::shared_ptr<dbcontroller> dbcontroller::getInstance() {
    std::lock_guard<std::mutex> guard(dbcontroller::instances_mutex);
    std::thread::id me = std::this_thread::get_id();
    std::shared_ptr<dbcontroller> inst;

    if (dbcontroller::thread_instances.count(me) == 0) {
        inst = std::shared_ptr<dbcontroller>(new dbcontroller());
        dbcontroller::thread_instances[me] = std::shared_ptr<dbcontroller>(inst);
        BOOSTER_DEBUG("scoreboard") << "Initializing dbc for thread " << std::this_thread::get_id();
    } else {
        inst = dbcontroller::thread_instances[me];
    }

    return std::shared_ptr<dbcontroller>(inst);
}

int dbcontroller::login(std::string tname, std::string password) {
    std::stringstream ss;
    ss.str("");
    ss << "login_" << tname << "$$$$" << password ;
    team t;
    t.valid = 0;

    if (cur_ctx->cache().fetch_data(ss.str(), t)) {
        return t.valid ? t.id : 0;
    }

    std::shared_ptr<QSqlQuery> stmt(new QSqlQuery(this->db));

    bool ok = doQuery(stmt, [&](std::shared_ptr<QSqlQuery> stmt) {
        if (!stmt->prepare("CALL verify_login(:nome,:psw,@idt);")) return false;
        stmt->bindValue("nome", QString::fromStdString(tname));
        stmt->bindValue("psw", QString::fromStdString(password));
        return true;
    });

    if (!ok) {
        BOOSTER_ERROR("scoreboard") << "verify_login() failed, aborting";
        return false;
    }

    stmt->exec("select @idt as id from dual;");

    if (!stmt->isActive()) {
        return false;
    }

    if ((stmt->size()) != 1) {
        cur_ctx->cache().store_data(ss.str(), t, -1);
        return false;
    } else {
        stmt->next();
        t.valid = 1;
        t.id = stmt->record().value(0).toInt();
        cur_ctx->cache().store_data(ss.str(), t, -1);
        return t.id;
    }
}

void dbcontroller::logIP(int teamid) {
    std::stringstream ss;
    ss.str("");
    ss << "teamip_" << teamid;
    teamip ti;
    std::string IP = cur_ctx->request().remote_addr();

    BOOSTER_NOTICE("scoreboard") << "team " << teamid << " has IP " << IP;
    // IN PRODUCTION ENV: string IP = cur_ctx->request().getenv("HTTP_X_FORWARDED_FOR"); // TO BE TESTED
    if (cur_ctx->cache().fetch_data(ss.str(), ti)) {
        for (std::vector<std::string>::iterator it = ti.seenip.begin(); it != ti.seenip.end(); ++it) {
            if (IP.compare(*it) == 0) {
                //already known IP for this team
                BOOSTER_DEBUG("scoreboard") << "found in cache.";
                return;
            }
        }
    }

    //Still not known. :(
    std::shared_ptr<QSqlQuery> stmt(new QSqlQuery(this->db));

    bool ok = doQuery(stmt, [&](std::shared_ptr<QSqlQuery> stmt) {
        if (!stmt->prepare("INSERT IGNORE INTO ipteams set idteam=:team, ip=:ip;")) return false;
        stmt->bindValue("team", teamid);
        stmt->bindValue("ip", QString::fromStdString(IP));
        return true;
    });

    if (!ok) {
        BOOSTER_ERROR("scoreboard") << "Could not add IP to teams IPs, aborting";
        return;
    }

    ok = doQuery(stmt, [&](std::shared_ptr<QSqlQuery> stmt) {
        if (!stmt->prepare("SELECT ip FROM ipteams WHERE idteam=:team;")) return false;
        stmt->bindValue("team", teamid);
        return true;
    });

    if (!ok) {
        BOOSTER_ERROR("scoreboard") << "Could not fetch team IP, aborting";
        return;
    }

    if (!ok) return;

    ti.id = teamid;
    ti.seenip.clear();

    while (stmt->next()) {
        ti.seenip.push_back(stmt->record().value(0).toString().toStdString());
        BOOSTER_DEBUG("scoreboard") << "Pushing in cache for team " << teamid << " value " << stmt->record().value(0).toString().toStdString();
    }

    cur_ctx->cache().store_data(ss.str(), ti, -1);
    return;
}

challengesol dbcontroller::fetchSolution(std::string const& flag) {
    std::string cachekey = "challenge_sol_" + flag;
    challengesol data;

    if (cur_ctx->cache().fetch_data(cachekey, data)) {
        return data;
    }

    data.valid = false;
    std::shared_ptr<QSqlQuery> stmt(new QSqlQuery(this->db));

    bool ok = doQuery(stmt, [&](std::shared_ptr<QSqlQuery> stmt) {
        if (!stmt->prepare("SELECT idchallenge, points from challenges where flaghash=sha2(:flag, 512) and opentime < NOW()")) return false;
        stmt->bindValue("flag", flag.c_str());
        return true;
    });

    if (!ok) {
        BOOSTER_ERROR("scoreboard") << "fetch solution failed, aborting";
        return data;
    }

    if ((stmt->size()) == 1) {
        stmt->next();
        data.valid = true;
        data.id = stmt->record().value("idchallenge").toInt();
        data.points = stmt->record().value("points").toInt();
    }

    cur_ctx->cache().store_data(
                cachekey, data,
                srv->settings().get<int>("cache.timeout.challengesol"));

    return data;
}

int dbcontroller::setChallengeSolved(int id) {
    int team = cur_ctx->session().get<int>("teamid");
    std::shared_ptr<QSqlQuery> stmt(new QSqlQuery(this->db));

    bool ok = doQuery(stmt, [&](std::shared_ptr<QSqlQuery> stmt) {
        if (!stmt->prepare("call SolvedChallenge(:team,:idchallenge,@points);")) return false;
        stmt->bindValue("team", team);
        stmt->bindValue("idchallenge", id);
        return true;
    });

    if (!ok) {
        BOOSTER_ERROR("scoreboard") << "SolvedChallenge() failed, aborting";
        return 0;
    }

    stmt->exec("select @points as points from dual;");
    std::stringstream trigger;
    trigger.str("");
    trigger << "teamsolved_" << team;
    cur_ctx->cache().rise(trigger.str());

    if ((stmt->size()) == 1) {
        stmt->next();
        return stmt->record().value("points").toInt();
    } else {
        BOOSTER_ERROR("scoreboard") << "Something went very wrong. SP SolvedChallenge returned " << stmt->size() << " records. Team " << team << " just solved challenge " << id;
        return 0;
    }
}

std::string dbcontroller::cachedQuery(int timeout, std::string cachekey, std::function<bool(std::shared_ptr<QSqlQuery>)> prepareQuery, std::function<void (cppcms::json::value &, std::shared_ptr<QSqlQuery>)> buildJSON) {
    return this->cachedQuery(timeout, [=](std::stringstream & ss) {
        ss << cachekey;
    }, prepareQuery, buildJSON);
}


std::string dbcontroller::cachedQuery(int timeout, std::function<void(std::stringstream &)> buildCacheKey, std::function<bool(std::shared_ptr<QSqlQuery>)> prepareQuery, std::function<void (cppcms::json::value &, std::shared_ptr<QSqlQuery>)> buildJSON) {
    cppcms::json::value ret = cppcms::json::value();
    std::stringstream ss;
    ss.str("");
    buildCacheKey(ss);
    std::string cachekey = ss.str();
    std::string output;

    if (cur_ctx->cache().fetch_frame(cachekey, output)) {
        return output;
    }

    //Has to be rebuilt!
    std::shared_ptr<QSqlQuery> stmt(new QSqlQuery(this->db));
    bool ok = doQuery(stmt, prepareQuery);

    if (ok) {
        buildJSON(ret, stmt);
        output = ret.save();
        cur_ctx->cache().store_frame(cachekey, output, makeStringSet(cachekey), timeout);
    }

    return output;
}

std::string dbcontroller::getGlobalWarnings() {
    return this->cachedQuery(
        srv->settings().get<int>("cache.timeout.globalwarnings"),
        "globalwarnings",
        [](std::shared_ptr<QSqlQuery> stmt) {
            return stmt->prepare("SELECT text,UNIX_TIMESTAMP(timestamp) as timestamp from alerts where recipient IS NULL;");
        },
        [](cppcms::json::value & ret, std::shared_ptr<QSqlQuery> stmt) {
            int i = 0;

            while (stmt->next()) {
                ret[i]["message"] = stmt->record().value("text").toString().toStdString();
                ret[i]["unixtime"] = stmt->record().value("timestamp").toInt();
                ++i;
            }

            if (i == 0) {
                ret = cppcms::json::array();
            }
        }
    );

}
std::string dbcontroller::getChallenge(int id) {
    return this->cachedQuery(
        srv->settings().get<int>("cache.timeout.challengetext"),
        [=](std::stringstream & ss) {
            ss << "challengetext_" << id;
        },
        [=](std::shared_ptr<QSqlQuery> stmt) {
            if (!stmt->prepare(
                "SELECT file, description, points, chall.name, cat.name as catname "
                    "from challenges as chall, categories as cat "
                    "where idchallenge=:id and hidden=0 and category=idcat and opentime < NOW();"
            )) return false;
            stmt->bindValue("id", id);
            return true;
        },
        [](cppcms::json::value & ret, std::shared_ptr<QSqlQuery> stmt) {
            if (stmt->size() != 1) {
                ret["html"] = "Error";
                return;
            }

            stmt->next();
            ret["html"] = stmt->record().value("description").toString().toStdString();
            ret["file"] = stmt->record().value("file").toString().toStdString();
            ret["name"] = stmt->record().value("name").toString().toStdString();
            ret["category"] = stmt->record().value("catname").toString().toStdString();
            ret["points"] = stmt->record().value("points").toInt();
        }
    );
}

std::string dbcontroller::getScores() {
    return this->cachedQuery(
        srv->settings().get<int>("cache.timeout.scores"), "scores",
        [](std::shared_ptr<QSqlQuery> stmt) {
            return stmt->prepare("SELECT * FROM classifica;");
        },
        [](cppcms::json::value & ret, std::shared_ptr<QSqlQuery> stmt) {
            int i = 0;

            while (stmt->next()) {
                ret[i]["name"] = stmt->record().value("name").toString().toStdString();
                ret[i]["points"] = stmt->record().value("points").toInt();
                ret[i]["country"] = stmt->record().value("country").toString().toStdString();
                ++i;
            }

            if (i == 0) {
                ret = cppcms::json::array();
            }
        }
    );
}

std::string dbcontroller::getCommonStatus() {
    return this->cachedQuery(
        srv->settings().get<int>("cache.timeout.commonstatus"), "status",
        [](std::shared_ptr<QSqlQuery> stmt) {
            return stmt->prepare("SELECT * FROM commonstatus;");
        },
        [](cppcms::json::value & ret, std::shared_ptr<QSqlQuery> stmt) {
            int i = 0;

            while (stmt->next()) {
                ret[i]["idchallenge"] = stmt->record().value("idchallenge").toInt();
                ret[i]["points"] = stmt->record().value("points").toInt();
                ret[i]["status"] = stmt->record().value("open").toInt() ? "open" : "closed";
                ret[i]["numsolved"] = stmt->record().value("numsolved").toInt();
                ret[i]["is_flash"] = stmt->record().value("is_flash").toInt();
                ++i;
            }

            if (i == 0) {
                ret = cppcms::json::array();
            }
        }
    );
}

std::string dbcontroller::getChallengesSummary() {
    return this->cachedQuery(
        srv->settings().get<int>("cache.timeout.challengesummary"), "challenges",
        [](std::shared_ptr<QSqlQuery> stmt) {
            return stmt->prepare(
                "select chall.idchallenge, chall.name as name, cat.name as category "
                    "from challenges chall, categories cat "
                    "where hidden = false and chall.category = cat.idcat and chall.opentime < now()");
        },
        [](cppcms::json::value & ret, std::shared_ptr<QSqlQuery> stmt) {
            if (stmt->size() == 0) {
                ret = cppcms::json::array();
                return;
            }

            while (stmt->next()) {
                std::string challid = stmt->record().value("idchallenge").toString().toStdString();
                ret[challid]["name"] = stmt->record().value("name").toString().toStdString();
                ret[challid]["cat"] = stmt->record().value("category").toString().toStdString();
            }
        }
    );
}

dbcontroller::~dbcontroller() {
    this->db.close();
}

teamSolutionList dbcontroller::fetchSolutionsForTeam(int teamId) {
    std::stringstream cacheKey;
    std::stringstream trigger;
    teamSolutionList solutions;
    cacheKey.str("");
    trigger.str("");
    cacheKey << "challenges_" << teamId;
    trigger << "teamsolved_" << teamId;
    std::shared_ptr<QSqlQuery> sqlQuery(new QSqlQuery(this->db));

    bool ok = doQuery(sqlQuery, [&](std::shared_ptr<QSqlQuery> stmt) {
        if (!stmt->prepare("SELECT solutions.idchallenge as idchallenge,earnedpoints,is_flash from solutions inner join challenges on (challenges.idchallenge=solutions.idchallenge) where idteam=:id")) return false;
        stmt->bindValue("id", teamId);
        return true;
    });

    if (!ok) {
        BOOSTER_ERROR("scoreboard") << "Could not fetch solutions for team, aborting";
        return {};
    }

    if (sqlQuery->size() == 0) {
        //LOL empty store, but let's not bother db until it's necessary
        cur_ctx->cache().store_data(
            cacheKey.str(), solutions,
            srv->settings().get<int>("cache.timeout.teamsolutions")
        );
        return solutions;
    }

    while (sqlQuery->next()) {
        teamSolution solution;
        solution.solutionPoints = sqlQuery->record().value("earnedpoints").toInt();
        solution.id = sqlQuery->record().value("idchallenge").toInt();
        solution.flashChallenge = sqlQuery->record().value("is_flash").toInt();
        solutions.challenges.push_back(solution);
    }

    cur_ctx->cache().store_data(
        cacheKey.str(), solutions, makeStringSet(trigger.str()),
        srv->settings().get<int>("cache.timeout.teamsolutions")
    );

    return solutions;
}

teamAlertList dbcontroller::fetchAlertsForTeam(int teamId) {
    std::stringstream cacheKey;
    teamAlertList alerts;
    cacheKey.str("");
    cacheKey << "alerts_" << teamId;
    std::shared_ptr<QSqlQuery> sqlQuery(new QSqlQuery(this->db));

    bool ok = doQuery(sqlQuery, [&](std::shared_ptr<QSqlQuery> stmt) {
        if (!stmt->prepare("SELECT text,UNIX_TIMESTAMP(timestamp) as timestamp,points from alerts where recipient=:id")) return false;
        stmt->bindValue("id", teamId);
        return true;
    });

    if (!ok) {
        BOOSTER_ERROR("scoreboard") << "Could not fetch alert for team, aborting";
        return {};
    }

    if (sqlQuery->size() == 0) {
        //LOL empty store, but let's not bother db until it's necessary
        cur_ctx->cache().store_data(
            cacheKey.str(), alerts,
            srv->settings().get<int>("cache.timeout.teamalerts")
        );
        // Returning empty alerts for team
        return alerts;
    }

    while (sqlQuery->next()) {
        teamAlert alert;
        alert.message = sqlQuery->record().value("text").toString().toStdString();
        alert.timestamp = sqlQuery->record().value("timestamp").toInt();
        alert.points = sqlQuery->record().value("points").toInt();
        alerts.alerts.push_back(alert);
    }

    cur_ctx->cache().store_data(
        cacheKey.str(), alerts,
        srv->settings().get<int>("cache.timeout.teamalerts")
    );

    return alerts;
}

teamInfo dbcontroller::fetchTeamInfo(int teamId) {
    std::stringstream cacheKey;
    std::stringstream trigger;
    teamInfo teamInfo;
    cacheKey.str("");
    cacheKey << "teaminfo_" << teamId;
    trigger.str("");
    trigger << "teamsolved_" << teamId;
    std::shared_ptr<QSqlQuery> sqlQuery(new QSqlQuery(this->db));

    bool ok = doQuery(sqlQuery, [&](std::shared_ptr<QSqlQuery> stmt) {
        if (!stmt->prepare("SELECT name,points from team where id=:id")) return false;
        stmt->bindValue("id", teamId);
        return true;
    });

    if (!ok) {
        BOOSTER_ERROR("scoreboard") << "Could not fetch info for team, aborting";
        return teamInfo;
    }

    if (sqlQuery->size() != 1) {
        //WTF?
        return teamInfo;
    }

    sqlQuery->next();
    teamInfo.name = sqlQuery->record().value("name").toString().toStdString();
    teamInfo.points = sqlQuery->record().value("points").toUInt();
    cur_ctx->cache().store_data(
        cacheKey.str(), teamInfo, this->makeStringSet(trigger.str()),
        srv->settings().get<int>("cache.timeout.teaminfo")
    );
    return teamInfo;
}

std::set<std::string> dbcontroller::makeStringSet(std::string str) {
    std::set<std::string> retval;
    retval.insert(str);
    return retval;
}

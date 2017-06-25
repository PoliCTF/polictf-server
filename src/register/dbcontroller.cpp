#include "dbcontroller.hpp"

#include <cppcms/json.h>
#include <cppcms/application.h>
#include <cppcms/http_context.h>
#include <cppcms/service.h>
#include <cppcms/serialization_classes.h>
#include <cppcms/serialization.h>
#include <cppcms/cache_interface.h>
#include <booster/log.h>

#include <QtSql>
#include <mysql/errmsg.h>

#include <sys/types.h>
#include <thread>
#include <memory>
#include <unistd.h>

#define DB_RECONNECT_MAX_SLEEP_TIME 4
#define MAX_TEAMS_PER_IP 4
#define DEFAULT_CACHE_TIMEOUT 300


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
        BOOSTER_ERROR("registration") << "Error while connecting to database: " << db.lastError().text().toStdString();
    }
}

std::mutex dbcontroller::instances_mutex;
std::unordered_map<std::thread::id,  std::shared_ptr<dbcontroller>> dbcontroller::thread_instances;

//return value: Should I retry now?
bool dbcontroller::handleError(std::shared_ptr<QSqlQuery> q) {
    int errnum = q->lastError().number();

    if (errnum == CR_SERVER_GONE_ERROR || errnum == CR_SERVER_LOST || !db.isOpen()) { //errori recuperabili riconnettendosi
        BOOSTER_ERROR("registration") << "Reopening connection to database.";
        db.close();
        sleep((rand() % DB_RECONNECT_MAX_SLEEP_TIME) + 1); //non DoSsiamo il server..
        db.open();
        return true;
    } else {
        std::stringstream ss;
        ss << "Error from database: ";
        ss << q->lastError().driverText().toStdString();
        ss << " (";
        ss << q->lastError().databaseText().toStdString();
        ss << ")";
        ss << "[" << q->executedQuery().toStdString() << "]";
        BOOSTER_ERROR("registration") << ss.str();
        //altri errori, potenzialmente dovuti alla query/dati/altri problemi sul db. Fermiamo la richiesta.
        return false;
    }
}


std::shared_ptr<dbcontroller> dbcontroller::getInstance() {
    std::thread::id me = std::this_thread::get_id();
     std::shared_ptr<dbcontroller> inst;
    dbcontroller::instances_mutex.lock();

    if (dbcontroller::thread_instances.count(me) == 0) {
        inst =  std::shared_ptr<dbcontroller>(new dbcontroller());
        dbcontroller::thread_instances[me] =  std::shared_ptr<dbcontroller>(inst);
    } else {
        inst = dbcontroller::thread_instances[me];
    }

    dbcontroller::instances_mutex.unlock();
    return  std::shared_ptr<dbcontroller>(inst);
}

bool dbcontroller::isSpammer() {
    std::string str = "";
    std::string ip = cur_ctx->request().remote_addr();
    std::stringstream ss;
    ss.str("");
    ss << "ipspammer_" << ip;

    if (cur_ctx->cache().fetch_frame(ss.str(), str)) {
        return (str == "t");
    }

    bool retval;
     std::shared_ptr<QSqlQuery> stmt(new QSqlQuery(this->db));

    do {
        stmt->prepare("SELECT COUNT(*) FROM team WHERE ip=:ip GROUP BY ip;");
        stmt->bindValue(":ip", QString::fromStdString(ip));
    } while (!stmt->exec() && handleError(stmt)); //un po acrobatico, ma non vedo controindicazioni :) se handleError ritorna true si può riprovare.

    if (!stmt->isActive()) {
        return true;
    }

    stmt->next();
    QSqlRecord r = stmt->record();
    retval = ((r.field(0).value().toInt()) > MAX_TEAMS_PER_IP);
    cur_ctx->cache().store_frame(ss.str(), (retval ? "t" : "f"), DEFAULT_CACHE_TIMEOUT);
    return retval;
}

bool dbcontroller::checkDuplicates(std::string name, std::string email) {
    std::string str = "";
    std::stringstream ss_name;
    std::stringstream ss_email;
    ss_name.str("");
    ss_name << "exists_name_" << name;
    ss_email.str("");
    ss_email << "exists_email_" << email;

    if (cur_ctx->cache().fetch_frame(ss_name.str(), str)) {
        if (str == "t") {
            return true;
        }
    }

    if (cur_ctx->cache().fetch_frame(ss_email.str(), str)) {
        if (str == "t") {
            return true;
        }
    }

    bool retval;
    std::shared_ptr<QSqlQuery> stmt(new QSqlQuery(this->db));

    do {
        stmt->prepare("SELECT name,email FROM team WHERE name=:nomesquadra or email=:email;");
        stmt->bindValue(":nomesquadra", QString::fromStdString(name));
        stmt->bindValue(":email", QString::fromStdString(email));
    } while (!stmt->exec() && handleError(stmt)); //un po acrobatico, ma non vedo controindicazioni :) se handleError ritorna true si può riprovare.

    if (!stmt->isActive()) {
        return true;
    }

    retval = ((stmt->size()) >= 1);

    if (retval) {
        stmt->next();
        QSqlRecord r = stmt->record();
        cur_ctx->cache().store_frame(ss_name.str(), ((r.field(QString("name")).value().toString().compare(QString::fromStdString(name))) ? "f" : "t"), DEFAULT_CACHE_TIMEOUT);
        cur_ctx->cache().store_frame(ss_email.str(), ((r.field(QString("email")).value().toString().compare(QString::fromStdString(email))) ? "f" : "t"), DEFAULT_CACHE_TIMEOUT);
    } else {
        cur_ctx->cache().store_frame(ss_name.str(), "f", DEFAULT_CACHE_TIMEOUT);
        cur_ctx->cache().store_frame(ss_email.str(), "f", DEFAULT_CACHE_TIMEOUT);
    }

    return retval;
}

bool dbcontroller::insert(frmIscrizione *frm , std::string *code) {
    std::shared_ptr<QSqlQuery> stmt(new QSqlQuery(this->db));

    do {
        /* removed sshkey */
        stmt->prepare("INSERT INTO team(name,email,password,website,twitter,ip,ua,confirmcode,country) VALUES(:name,:email,SHA2(:password,512),:website,:twitter,:ip,:ua,:confirmcode,:country);");
        stmt->bindValue(":name", QString::fromStdString(frm->name.value()));
        stmt->bindValue(":email", QString::fromStdString(frm->email.value()));
        stmt->bindValue(":password", QString::fromStdString(frm->password.value()));
        /* stmt->bindValue("sshkey", QString::fromStdString(frm->sshkey.value())); */
        stmt->bindValue(":website", QString::fromStdString(frm->website.value()));
        stmt->bindValue(":twitter", QString::fromStdString(frm->twitter.value()));
        stmt->bindValue(":ip", QString::fromStdString(cur_ctx->request().remote_addr()));
        stmt->bindValue(":ua", QString::fromStdString(cur_ctx->request().http_user_agent()));
        stmt->bindValue(":confirmcode", QString::fromStdString(*code));
        stmt->bindValue(":country", QString::fromStdString(frm->country.value()));
        /* stmt->bindValue(":size", QString::number(frm->size.value())); */
        /* stmt->bindValue(":age_y", QString::number(frm->age_youngest.value())); */
        /* stmt->bindValue(":age_o", QString::number(frm->age_oldest.value())); */
    } while (!stmt->exec() && handleError(stmt));

    std::stringstream ss;
    ss.str("");
    ss << "exists_name_" << frm->name.value();
    cur_ctx->cache().rise(ss.str());
    ss.str("");
    ss << "exists_email_" << frm->email.value();
    cur_ctx->cache().rise(ss.str());
    ss.str("");
    ss << "ipspammer_" << cur_ctx->request().remote_addr();
    cur_ctx->cache().rise(ss.str());
    return ((stmt->numRowsAffected()) == 1);
}

bool dbcontroller::initReset(frmPasswordRecoveryInit *frm, std::string *token) {
    std::shared_ptr<QSqlQuery> stmt(new QSqlQuery(this->db));

    do {
        stmt->prepare("UPDATE team SET reset_token=:token, reset_timestamp=NOW() WHERE email=:email;");
        stmt->bindValue(":token", QString::fromStdString(*token));
        stmt->bindValue(":email", QString::fromStdString(frm->email.value()));
    } while(!stmt->exec() && handleError(stmt));

    return stmt->numRowsAffected() == 1;
}

bool dbcontroller::resetPassword(frmPasswordRecovery *frm) {
    std::shared_ptr<QSqlQuery> stmt(new QSqlQuery(this->db));

    do {
        stmt->prepare("UPDATE team SET password=SHA2(:password, 512), reset_token=NULL WHERE email=:email AND reset_token=:token AND reset_timestamp > DATE_SUB(NOW(), INTERVAL 1 HOUR) AND reset_token is not null;");
        stmt->bindValue(":password", QString::fromStdString(frm->password.value()));
        stmt->bindValue(":email", QString::fromStdString(frm->email.value()));
        stmt->bindValue(":token", QString::fromStdString(frm->token.value()));
    } while(!stmt->exec() && handleError(stmt));

    return stmt->numRowsAffected() == 1;
}

dbcontroller::~dbcontroller() {
    this->db.close();
}

bool dbcontroller::confirm(std::string *code) {
    std::shared_ptr<QSqlQuery> stmt(new QSqlQuery(this->db));
    bool retv;
    std::string str = "";
    std::stringstream ss;
    ss.str("");
    ss << "confirmkey_" << *code;

    if (cur_ctx->cache().fetch_frame(ss.str(), str)) {
        return false;
    }

    do {
        stmt->prepare("UPDATE team SET confirmip=:ip, confirmua=:ua, confirm_timestamp=NOW() WHERE confirmcode=:code AND confirmip is null;");
        stmt->bindValue(":ip", QString::fromStdString(cur_ctx->request().remote_addr()));
        stmt->bindValue(":ua", QString::fromStdString(cur_ctx->request().http_user_agent()));
        stmt->bindValue(":code", QString::fromStdString(*code));
    } while (!stmt->exec() && handleError(stmt));

    if (!stmt->isActive()) {
        return false;
    }

    retv = ((stmt->numRowsAffected()) == 1);
    cur_ctx->cache().store_frame(ss.str(), *code, DEFAULT_CACHE_TIMEOUT);
    return retv;
}

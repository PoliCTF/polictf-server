#ifndef SCOREBOARD_DBCONTROLLER_H
#define SCOREBOARD_DBCONTROLLER_H

#include "cache_objects.hpp"

#include <memory>
#include <mutex>
#include <thread>
#include <string>
#include <exception>

#include <cppcms/http_context.h>
#include <cppcms/json.h>

#include <QtSql>


class loginException: public std::runtime_error {
    public:
        loginException(std::string const& msg):
            std::runtime_error(msg)
        {}
};

class dbException: public std::runtime_error {
    public:
        dbException(std::string const& msg):
            std::runtime_error(msg)
    {}
};

class dbcontroller {
public:
    static std::shared_ptr<dbcontroller> getInstance();

    ~dbcontroller();

    std::string getCommonStatus();
    std::string getScores();
    std::string getGlobalWarnings();
    std::string getChallengesSummary();
    std::string getChallenge(int id);
    int setChallengeSolved(int id);

    challengesol fetchSolution(std::string const& flag);
    teamSolutionList fetchSolutionsForTeam(int teamId);
    teamAlertList fetchAlertsForTeam (int teamId);
    teamInfo fetchTeamInfo(int teamId);

    int login(std::string team, std::string password);
    bool confirm(std::string*code);
    void logIP(int teamid);

protected:
    dbcontroller();

    bool handleError(QSqlError err);
    bool doQuery(std::shared_ptr<QSqlQuery> stmt, std::function<bool(std::shared_ptr<QSqlQuery>)> prepare, int maxretries=10);

    std::string cachedQuery(int timeout, std::string cachekey, std::function<bool (std::shared_ptr<QSqlQuery>)> prepareQuery, std::function<void (cppcms::json::value&, std::shared_ptr<QSqlQuery>)> buildJSON);
    std::string cachedQuery(int timeout, std::function<void(std::stringstream&)> buildCacheKey, std::function<bool(std::shared_ptr<QSqlQuery>)> prepareQuery, std::function<void(cppcms::json::value&, std::shared_ptr<QSqlQuery>)> buildJSON);

    QSqlDatabase db;
    std::set<std::string> makeStringSet(std::string str);

    static std::unordered_map<std::thread::id, std::shared_ptr<dbcontroller>> thread_instances;
    static std::mutex instances_mutex;
};

#endif // SCOREBOARD_DBCONTROLLER_H

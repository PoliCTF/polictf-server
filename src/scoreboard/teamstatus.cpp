#include "teamstatus.hpp"

#include <cppcms/session_interface.h>
#include <mutex>


extern __thread cppcms::http::context *cur_ctx;

TeamStatus::~TeamStatus() {}

TeamStatus::TeamStatus(int _teamid) {
    teamid = _teamid;
    std::stringstream tmp;
    tmp.str("");
    tmp << "challenges_" << teamid;
    std::string cachekey = tmp.str();
    std::shared_ptr<dbcontroller> dbc;

    if (!cur_ctx->cache().fetch_data(cachekey, solutions)) {
        dbc = dbcontroller::getInstance();
        solutions = dbc->fetchSolutionsForTeam(teamid);
    }

    tmp.str("");
    tmp << "alerts_" << teamid;
    cachekey = tmp.str();

    if (!cur_ctx->cache().fetch_data(cachekey, alerts)) {
        dbc = dbcontroller::getInstance();
        alerts = dbc->fetchAlertsForTeam(teamid);
    }

    tmp.str("");
    tmp << "teaminfo_" << teamid;
    cachekey = tmp.str();

    if (!cur_ctx->cache().fetch_data(cachekey, info)) {
        dbc = dbcontroller::getInstance();
        info = dbc->fetchTeamInfo(teamid);
    }
}

std::string TeamStatus::getJson() {
    cppcms::json::value object;
    int challs_count = solutions.challenges.size();
    int flashchall_points = 0;
    int flashchall_index = 0;

    for (int chall_index = 0; chall_index < challs_count; chall_index++) {
        if (solutions.challenges.at(chall_index).flashChallenge) {
            object["statosquadra"]["flashch_solved"][flashchall_index] = solutions.challenges.at(chall_index).id;
            flashchall_points += solutions.challenges.at(chall_index).solutionPoints;
            ++flashchall_index;
            continue;
        }

        object["solved"][chall_index]["points"] = solutions.challenges.at(chall_index).solutionPoints;
        object["solved"][chall_index]["id"] = solutions.challenges.at(chall_index).id;
    }

    object["statosquadra"]["flashch_points"] = flashchall_points;
    object["statosquadra"]["nome"] = info.name;
    object["statosquadra"]["totpoints"] = info.points;
    challs_count = alerts.alerts.size();

    for (int alert_index = 0; alert_index < challs_count; alert_index++) {
        object["teamwarnings"][alert_index]["message"] = alerts.alerts.at(alert_index).message;
        object["teamwarnings"][alert_index]["unixtime"] = alerts.alerts.at(alert_index).timestamp;
        object["teamwarnings"][alert_index]["points"] = alerts.alerts.at(alert_index).points;
    }

    return object.save();
}


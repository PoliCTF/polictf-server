#ifndef SCOREBOARD_TEAMSTATUS_H
#define SCOREBOARD_TEAMSTATUS_H

#include "cache_objects.hpp"
#include "dbcontroller.hpp"

#include <memory>

#include <openssl/evp.h>

#include <cppcms/application.h>
#include <cppcms/http_context.h>
#include <cppcms/service.h>
#include <cppcms/http_request.h>
#include <cppcms/cache_interface.h>
#include <cppcms/serialization.h>
#include <cppcms/session_interface.h>


class TeamStatus {
public:
    TeamStatus(int _teamid);
    ~TeamStatus();

    std::string getJson();

private:
    int teamid;
    teamInfo info;
    teamAlertList alerts;
    teamSolutionList solutions;
};

#endif // SCOREBOARD_TEAMSTATUS_H

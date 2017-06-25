#include "dbcontroller.hpp"
#include "verifier.hpp"
#include "teamstatus.hpp"

#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_context.h>
#include <cppcms/http_response.h>
#include <cppcms/serialization.h>
#include <cppcms/session_interface.h>
#include <cppcms/cache_interface.h>
#include <cppcms/url_dispatcher.h>
#include <cppcms/json.h>
#include <cppcms/util.h>
#include <cppcms/http_request.h>
#include <booster/log.h>

#include <memory>
#include <mutex>
#include <exception>
#include <string>

__thread cppcms::http::context *cur_ctx = NULL;

#define ASSERT_METHOD(method) if (!checkMethod(#method)) return
#define HANDLECORSPREFLIGHT if(handleCORSPreflight()) return

//c'è un istanza di questa classe per ogni thread.
class scoreboard: public cppcms::application {
public:
    scoreboard(cppcms::service &srv): cppcms::application(srv) {
        dispatcher().assign("/common/status", &scoreboard::commonstatus, this);
        dispatcher().assign("/common/challenge/([0-9]+)", &scoreboard::challenge, this, 1);
        dispatcher().assign("/common/challenges", &scoreboard::challenges, this);
        dispatcher().assign("/team/status", &scoreboard::teamstatus, this);
        dispatcher().assign("/team/submit", &scoreboard::teamsubmit, this);
        dispatcher().assign("/login", &scoreboard::login, this);
        dispatcher().assign("/logout", &scoreboard::logout, this);
    }

    void commonstatus() {
        ASSERT_METHOD(GET);

        cacheable();
        withCORS(Credentials::NO);
        initResponse(LoggedIn::NO);

        std::shared_ptr<dbcontroller> db = dbcontroller::getInstance();

        response().out() <<
            "{"
            "\"globalwarnings\":"
                << db->getGlobalWarnings() << ","
            "\"status\":"
                << db->getCommonStatus() << ","
            "\"scores\":"
                << db->getScores() <<
            "}";
    }

    void challenge(std::string idchallenge) {
        ASSERT_METHOD(GET);

        cacheable();
        withCORS(Credentials::NO);
        initResponse(LoggedIn::NO);

        int id = std::stoi(idchallenge.c_str());
        std::shared_ptr<dbcontroller> db = dbcontroller::getInstance();
        response().out() << db->getChallenge(id);
    }

    void challenges() {
        ASSERT_METHOD(GET);

        cacheable();
        withCORS(Credentials::NO);
        initResponse(LoggedIn::NO);

        std::shared_ptr<dbcontroller> db = dbcontroller::getInstance();
        response().out() << db->getChallengesSummary();
    }

    void teamstatus() {
        ASSERT_METHOD(GET);

        int teamid = initResponse(LoggedIn::YES);
        noncacheable();
        withCORS(Credentials::YES);

        if (!teamid) {
            notauth();
            return;
        };

        TeamStatus tstatus(teamid);

        response().out() << tstatus.getJson();
    }

    void teamsubmit() {
        HANDLECORSPREFLIGHT;
        ASSERT_METHOD(POST);

        noncacheable();
        withCORS(Credentials::YES);
        int teamid = initResponse(LoggedIn::YES);

        if (!teamid) {
            notauth();
            return;
        };

        BOOSTER_NOTICE("scoreboard")
            << "Received flag from "
            << cur_ctx->request().remote_addr()
            << " (teamid = " << teamid << "): `"
            << request().post("flag") << '`';
        ;

        verifier v;
        verifier::response res = v.verify(request().post("flag"));

        cppcms::json::value ret;
        ret["id"] = res.challengeid;
        ret["flag"] = cppcms::util::escape(request().post("flag"));

        switch (res.points) {
        case verifier::ALREADY_SOLVED:
            ret["result"] = "alreadysolved";
            break;
        case verifier::WRONG:
            ret["result"] = "wrong";
            break;
        case verifier::CANNOTSAVE:
            ret["result"] = "rightbutcannotsave";
            break;
        case verifier::TOO_FAST:
            ret["result"] = "slowdown";
            break;
        default:
            ret["result"] = res.points;
        }

        std::string json_response = ret.save();

        BOOSTER_NOTICE("scoreboard")
            << "Successfully processed submission from "
            << cur_ctx->request().remote_addr()
            << " (teamid = " << teamid << "): "
            << json_response;
        ;

        response().out() << json_response;
    }

    void login() {
        HANDLECORSPREFLIGHT;
        ASSERT_METHOD(POST);

        noncacheable();
        withCORS(Credentials::YES);

        initResponse(LoggedIn::NO);
        std::shared_ptr<dbcontroller> db = dbcontroller::getInstance();

        session().clear();
        session().reset_session();

        try {
            int tid = db->login(request().post("teamname"), request().post("password"));

            if (tid > 0) {
                session().set("teamid", tid);
                response().out() << "{\"r\":1}";
            } else {
                response().out() << "{\"r\":0, \"reason\":\"Wrong username or password.\"}";
            }
        } catch(dbException& dbe) {
            response().out() << "{\"r\":0, \"reason\":\"Ouch! A technical error occurred during login. Please try again later.\"}";
        } catch(loginException& le) {
            response().out() << "{\"r\":0, \"reason\":\"" << le.what() << "\"}";
        }
    }

    void logout() {
        HANDLECORSPREFLIGHT;
        ASSERT_METHOD(POST);

        noncacheable();
        withCORS(Credentials::YES);

        if (!initResponse(LoggedIn::YES)) {
            notauth();
            return;
        }

        session().clear();
        session().reset_session();
        response().out() << "{\"r\":1}";
    }

private:
    enum class Credentials {
        YES, NO
    };

    enum class LoggedIn {
        YES, NO
    };

    int initResponse(LoggedIn checklogin) {
        cur_ctx = &context();

        response().content_type("application/json");

        //And now log the IP
        if (checklogin == LoggedIn::NO) {
            return false;
        }

        int teamid = false;

        if (session().is_set("teamid")) {
            teamid = session().get<int>("teamid");

            if (teamid <= 0) {
                teamid = false;
            } else {
                std::shared_ptr<dbcontroller> db = dbcontroller::getInstance();
                db->logIP(teamid);
            }
        }

        return teamid;
    }

    bool checkMethod(std::string const& method) {
        if (request().request_method() != method) {
            response().make_error_response(400, "Wrong method");
            return false;
        }
        return true;
    }

    // returns true if the request was a OPTIONS one, and it's been handled
    bool handleCORSPreflight(){
         if (request().request_method() == "OPTIONS") {
            response().set_header(
                "Access-Control-Allow-Origin", settings().get<std::string>("service.static-domain")
            );
            response().set_header("Access-Control-Allow-Credentials", "true");
            response().set_header("Access-Control-Allow-Headers", "Content-Type");
            return true;
         }
         return false;
    }

    void withCORS(Credentials credentials) {
        response().set_header(
            "Access-Control-Allow-Origin", settings().get<std::string>("service.static-domain")
        );
        if (credentials == Credentials::YES) {
            response().set_header("Access-Control-Allow-Credentials", "true");
        }
    }

    void notauth() {
        response().out() << "{\"status\":\"Plz login.\"}";
    }

    void cacheable() {
        response().cache_control(settings().get<std::string>("cache.cacheable-header"));
    }

    void noncacheable() {
        response().cache_control(settings().get<std::string>("cache.non-cacheable-header"));
    }
};

cppcms::service *srv = NULL;

int main(int argc, char **argv) {
    BOOSTER_INFO("scoreboard") << "Starting up";

    try {
        srv = new cppcms::service(argc, argv);
        srv->applications_pool().mount(cppcms::applications_factory<scoreboard>());
        srv->run();
        delete srv;
    } catch (std::exception const& e) {
        BOOSTER_CRITICAL("scoreboard") << e.what();
    }
}

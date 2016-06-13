#include "verifier.hpp"
#include "cache_objects.hpp"
#include "dbcontroller.hpp"

#include <cppcms/application.h>
#include <cppcms/http_context.h>
#include <cppcms/service.h>
#include <cppcms/http_request.h>
#include <cppcms/cache_interface.h>
#include <cppcms/serialization.h>
#include <cppcms/session_interface.h>
#include <booster/log.h>

#include <memory>
#include <mutex>
#include <cassert>
#include <string>


extern __thread cppcms::http::context *cur_ctx;
extern cppcms::service *srv;

verifier::response verifier::verify(std::string const& flag) {
    std::string teamid = std::to_string(cur_ctx->session().get<int>("teamid"));
    std::string team_last_submission_ck = "last_submission_" + teamid;
    std::string ignored;

    // 1. Check if last submission was less than X seconds ago. If yes, rate limit.
    if (cur_ctx->cache().fetch_frame(team_last_submission_ck, ignored)) {
        BOOSTER_INFO("scoreboard")
            << "Rate limit triggered for teamid=" << teamid
            << ", submission was: `" << flag << "`"
        ;
        return {TOO_FAST, -1};
    }

    // 2. Set rate limit cookie to expire after X seconds
    cur_ctx->cache().store_frame(
                team_last_submission_ck, "",
                srv->settings().get<int>("service.rate-limit-seconds"));

    // 3. Fetch challenge solution associated with given flag, if any
    std::shared_ptr<dbcontroller> db = dbcontroller::getInstance();
    challengesol data = db->fetchSolution(flag);

    // 4. If there was no valid challenge associated, bail out
    if (data.valid != true) {
        BOOSTER_DEBUG("scoreboard") << "Data retrieved from DB is not valid";
        return {WRONG, -1};
    }

    // 5. Check if the team already solved the challenge. If yes, bail out
    std::string team_solution_ck = "chsolv_" + std::to_string(data.id) + "_" + teamid;
    if (cur_ctx->cache().fetch_frame(team_solution_ck, ignored)) {
        return {ALREADY_SOLVED, data.id};
    }

    // 6. Check if progress saving is disabled in settings.
    if (!srv->settings().get<bool>("service.allow-save-solved")) {
        return {CANNOTSAVE, data.id};
    }

    // 7. Set cookie tested at point 5. for already solved.
    cur_ctx->cache().store_frame(team_solution_ck, "", -1);

    // 8. Invoke points calculation and storage procedure
    int pts = db->setChallengeSolved(data.id);

    if (pts == -1) {
        // Already solved, but cache had expired (e.g. daemon restart)
        return {ALREADY_SOLVED, data.id};
    }

    // 9. Invalidate all dependent caches
    cur_ctx->cache().rise("teamsolved_" + teamid);
    cur_ctx->cache().rise("scores");

    if (pts > data.points) {
        cur_ctx->cache().rise("status");
    }

    // 10. We are done! Goodbye!
    return {pts, data.id};
}

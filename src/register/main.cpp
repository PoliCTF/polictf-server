#include "frmIscrizione.hpp"
#include "dbcontroller.hpp"
#include "mailer.hpp"

#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_context.h>
#include <cppcms/http_response.h>
#include <cppcms/url_dispatcher.h>
#include <booster/log.h>

#include <QByteArray>

#include <memory>
#include <fstream>
#include <exception>

/**
 * @brief generate a token containing the specified number of random bytes
 * @param size number of random bytes for the token
 * @return the token or an empty string if something failed.
 *
 * The token is base-64 encoded, so it will be larger than the specified number
 * of random bytes. `len` is effectively a strength parameter, not the length
 * of the output string.
 */
static
std::string randstr(unsigned size) {
    std::ifstream urandom("/dev/urandom");
    std::unique_ptr<char> random_data {new char[size]};

    if (random_data == nullptr) {
        BOOSTER_ERROR("registration") << "Cannot allocate memory for token.";
        return "";
    }

    if (!urandom.read(random_data.get(), size)) {
        BOOSTER_ERROR("registration") << "Could not read from random device";
        return "";
    }

    return std::string {
        QByteArray(random_data.get(), size)
            .toBase64()
            .replace('+', '-')
            .replace('/', '_')
            .data()
    };
}

__thread cppcms::http::context *cur_ctx = NULL;

#define ASSERT_METHOD(method) if (!checkMethod(#method)) return

//c'è un istanza di questa classe per ogni thread.
class iscrizione: public cppcms::application {
public:
    iscrizione(cppcms::service &srv): cppcms::application(srv) {
        dispatcher().assign("/subscribe", &iscrizione::subscribe, this);
        dispatcher().assign("/confirm/([a-zA-Z0-9_-]+)", &iscrizione::confirm, this, 1);
    }

    void confirm(std::string code) {
        ASSERT_METHOD(GET);

        cur_ctx = &context();
        std::shared_ptr<dbcontroller> dbc = dbcontroller::getInstance();

        if (dbc->confirm(&code)) {
            response().location(settings().get<std::string>("url.confirm-ok"));
        } else {
            response().location(settings().get<std::string>("url.confirm-ko"));
        }

        response().make_error_response(302, "Redirect");
    }

    void subscribe() {
        ASSERT_METHOD(POST);

        mailer &mail = mailer::getInstance();
        cur_ctx = &context();
        cppcms::json::value reply;
        std::shared_ptr<dbcontroller> dbc = dbcontroller::getInstance();
        std::string uniqid = randstr(18);
        if (uniqid.empty()) {
            response().make_error_response(503, "The server is mumbling");
            return;
        }
        frmIscrizione frm;
        frm.load(context());
        reply["done"] = false;

        response().set_header(
            "Access-Control-Allow-Origin", settings().get<std::string>("service.static-domain")
        );
        response().content_type("application/json");

        if (dbc->isSpammer()) {
            reply["errors"][0] =  "You're registering too many teams.";
        } else if (!frm.validate()) {
            reply["errors"] =  frm.errors;
        } else if (!dbc->insert(&frm, &uniqid)) {
            reply["errors"][0] =  "There was a problem during DB access.";
        } else {
            mail.sendEmail(uniqid, frm);
            reply["done"] = true;
        }

        response().out() << reply;
    }

private:
    bool checkMethod(std::string const& method) {
        if (request().request_method() != method) {
            response().make_error_response(400, "Wrong method");
            return false;
        }
        return true;
    }
};

cppcms::service *srv = NULL;

int main(int argc, char **argv) {
    try {
        srv = new cppcms::service(argc, argv);
        srv->applications_pool().mount(cppcms::applications_factory<iscrizione>());
        srv->run();
        delete srv;
    } catch (std::exception const& e) {
        BOOSTER_CRITICAL("registration") << e.what();
    }
}

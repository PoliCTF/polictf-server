#include "frmPasswordRecovery.hpp"
#include "dbcontroller.hpp"

#include <cppcms/form.h>
#include <memory>


using namespace cppcms;

frmPasswordRecovery::frmPasswordRecovery():
    errors(),
    email(),
    token("[a-zA-Z0-9_-]+"),
    password("^.*(?=.{8,90})(?=.*\\d.*)(?=.*[a-zA-Z].*).*$")
{
    email.name("email");
    token.name("token");
    password.name("password");

    add(email);
    add(token);
    add(password);
}

frmPasswordRecovery::~frmPasswordRecovery() {}

bool frmPasswordRecovery::validate() {
    std::shared_ptr<dbcontroller> dbc = dbcontroller::getInstance();

    if (!email.validate()) {
        errors.push_back("Please insert the team email");
    }

    if (!password.validate()) {
        errors.push_back("Please type a password between 8 and 90 characters, containing letters and numbers.");
    }

    if (!token.validate()) {
        errors.push_back("Token not valid.");
    }

    return form::validate();
}

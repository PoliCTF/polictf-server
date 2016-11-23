#include "frmPasswordRecoveryInit.hpp"
#include "dbcontroller.hpp"

#include <cppcms/form.h>
#include <memory>


using namespace cppcms;

frmPasswordRecoveryInit::frmPasswordRecoveryInit():
    errors(),
    email()
{
    email.name("email");

    add(email);
}

frmPasswordRecoveryInit::~frmPasswordRecoveryInit() {}

bool frmPasswordRecoveryInit::validate() {

    if (!email.validate()) {
        errors.push_back("Please type a valid email address.");
        return false;
    }

    return true;
}

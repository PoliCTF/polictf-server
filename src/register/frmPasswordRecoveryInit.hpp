#ifndef REGISTER_FRMPASSWORDRECOVERY_H
#define REGISTER_FRMPASSWORDRECOVERY_H

#include <cppcms/form.h>

class frmPasswordRecoveryInit : public cppcms::form {
public:
    frmPasswordRecoveryInit();
    virtual ~frmPasswordRecoveryInit();

    virtual bool validate();

    std::vector<std::string> errors;
    cppcms::widgets::email email;
};

#endif

#ifndef REGISTER_FRMRECOVERY_H
#define REGISTER_FRMRECOVERY_H

#include <cppcms/form.h>

class frmPasswordRecovery : public cppcms::form {
public:
    frmPasswordRecovery();
    virtual ~frmPasswordRecovery();

    virtual bool validate();

    std::vector<std::string> errors;
    cppcms::widgets::email email;
    cppcms::widgets::regex_field token;
    cppcms::widgets::regex_field password;
};

#endif

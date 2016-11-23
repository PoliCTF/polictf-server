#ifndef REGISTER_FRMISCRIZIONE_H
#define REGISTER_FRMISCRIZIONE_H

#include <cppcms/form.h>

class frmIscrizione : public cppcms::form {
public:
    frmIscrizione();
    virtual ~frmIscrizione();

    virtual bool validate();

    std::vector<std::string> errors;
    cppcms::widgets::regex_field name, password, country;
    cppcms::widgets::regex_field website;
    cppcms::widgets::checkbox rules;
    cppcms::widgets::email email;
    /* cppcms::widgets::regex_field sshkey; */
    cppcms::widgets::numeric<int> size, age_youngest, age_oldest;
};

#endif // FRMISCRIZIONE_H

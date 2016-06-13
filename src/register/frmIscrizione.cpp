#include "frmIscrizione.hpp"
#include "dbcontroller.hpp"

#include <cppcms/form.h>
#include <memory>


using namespace cppcms;

frmIscrizione::frmIscrizione():
    errors(),
    name("^[a-zA-Z0-9 -_]{3,25}$"),
    password("^.*(?=.{8,90})(?=.*\\d.*)(?=.*[a-zA-Z].*).*$"),
    country("^[a-zA-Z .,()]*$"),
    website("^(?:https?://.+)?$"),
    rules(),
    email(), //è un widget di tipo email, quindi non serve specificare una regexp
    sshkey("^ssh-(rsa|dsa|ecdsa) [a-zA-Z/0-9+=]{3,}\\s*[@a-zA-Z0-9-_]*$"),
    size(),
    age_youngest(),
    age_oldest()
{
    name.name("name");
    email.name("email");
    password.name("password");
    sshkey.name("sshkey");
    rules.name("rules");
    country.name("country");
    website.name("website");
    size.name("size");
    age_youngest.name("age_youngest");
    age_oldest.name("age_oldest");

    add(name);
    add(email);
    add(password);
    add(rules);
    add(country);
    add(website);

    add(sshkey);
    add(size);
    add(age_youngest);
    add(age_oldest);
}

frmIscrizione::~frmIscrizione() {}

bool frmIscrizione::validate() {
    std::shared_ptr<dbcontroller> dbc = dbcontroller::getInstance();

    if (!name.validate()) {
        errors.push_back("Please type a team name between 6 and 25 characters among letters, numbers, _ and -");
    }

    if (!password.validate()) {
        errors.push_back("Please type a password between 8 and 90 characters, containing letters and numbers.");
    }

    if (!email.validate()) {
        errors.push_back("Please type a valid email address.");
    }

    if (!country.validate()) {
        errors.push_back("Please provide a valid country name");
    }
    if (!sshkey.validate()) {
        errors.push_back("Please type a valid ssh public key in format ssh-rsa KEY [name].");
    }

    if (!website.validate()) {
        errors.push_back("Please type a valid URL.");
    }

    if (!form::validate()) {
        return false;
    }

    if (!sshkey.set()) {
        sshkey.value("");
    }

    if (!size.set()) {
        size.value(-1);
    }

    if (!age_youngest.set()) {
        age_youngest.value(-1);
    }

    if (!age_oldest.set()) {
        age_oldest.value(-1);
    }

    if (!website.set()) {
        website.value("");
    }

    if (!rules.value()) {
        errors.push_back("Please accept our rules!");
        return false;
    }

    if (dbc->checkDuplicates(name.value(), email.value())) {
        errors.push_back("Looks like you're already registered. Please choose another name and/or change email address.");
        return false;
    }

    return true;
}

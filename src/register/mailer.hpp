#ifndef REGISTER_MAILER_H
#define REGISTER_MAILER_H

#include "frmIscrizione.hpp"

#include <cppcms/json.h>
#include <cppcms/service.h>
#include <QtCore>


class mailer {
public:
    static mailer &getInstance();
    void sendEmail(std::string &uid, frmIscrizione &frm);

protected:
    mailer();

    QString smtphost;
    QString smtpuser;
    QString smtppsw;
    QString mailfromaddr;
    QString mailfromname;
    QString subject;
    QString text;
    bool smtpauth;
};

#endif

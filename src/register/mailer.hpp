#ifndef REGISTER_MAILER_H
#define REGISTER_MAILER_H

#include "frmIscrizione.hpp"
#include "frmPasswordRecoveryInit.hpp"

#include "smtp-qt/emailaddress.h"

#include <cppcms/json.h>
#include <cppcms/service.h>
#include <QtCore>


class mailer {
public:
    static mailer &getInstance();
    bool sendConfirmEmail(std::string &uid, frmIscrizione &frm);
    bool sendPasswordRecoveryEmail(std::string &uid, frmPasswordRecoveryInit &frm);

protected:
    mailer();

    QString smtphost;
    QString smtpuser;
    QString smtppsw;
    QString mailfromaddr;
    QString mailfromname;
    bool smtpauth;

    QString confirm_registration_text;
    QString confirm_registration_subject;

    QString password_recovery_text;
    QString password_recovery_subject;

    std::string password_recovery_url;

private:
    bool send(QString& txt, QString& subject, EmailAddress& receiver);

};

#endif

#include "mailer.hpp"
#include "smtp-qt/smtpclient.h"
#include "smtp-qt/mimetext.h"

#include <QtCore>
#include <memory>


extern cppcms::service *srv;

mailer &mailer::getInstance() {
    static mailer me;
    return me;
}

mailer::mailer():
    smtphost(), smtpuser(), smtppsw(), mailfromaddr(), mailfromname() {
    cppcms::json::value settings = srv->settings();
    this->smtphost = QString::fromStdString(settings.get<std::string>("smtp.host"));
    this->smtpuser = QString::fromStdString(settings.get<std::string>("smtp.user"));
    this->smtppsw = QString::fromStdString(settings.get<std::string>("smtp.password"));

    if (settings.get<int>("smtp.auth") >= 1) {
        this->smtpauth = true;
    } else {
        this->smtpauth = false;
    }

    this->mailfromaddr = QString::fromStdString(settings.get<std::string>("mail.from_address"));
    this->mailfromname = QString::fromStdString(settings.get<std::string>("mail.from_name"));

    this->confirm_registration_text = QString::fromStdString(settings.get<std::string>("mail.confirm_registration.text"));
    this->confirm_registration_subject = QString::fromStdString(settings.get<std::string>("mail.confirm_registration.subject"));

    this->password_recovery_text = QString::fromStdString(settings.get<std::string>("mail.password_recovery.text"));
    this->password_recovery_subject = QString::fromStdString(settings.get<std::string>("mail.password_recovery.subject"));

    this->password_recovery_url = settings.get<std::string>("url.password-recovery");

}

std::shared_ptr<QString> ReplaceValues(QString input, QHash<QString, QString> *replacements) {
    std::unique_ptr<QRegExp> rx(new QRegExp("!\\{([^}]+)\\}"));
    std::shared_ptr<QString> outp(new QString(input));
    int pos = 0;

    while ((pos = rx->indexIn(input, pos)) != -1) {
        if (replacements->contains(rx->cap(1))) {
            outp->replace(QString("!{%1}").arg(rx->cap(1)), replacements->value(rx->cap(1)));
        }

        pos += rx->matchedLength();
    }

    return std::shared_ptr<QString>(outp);
}

bool mailer::send(QString& txt, QString& subject, EmailAddress& receiver) {
	std::unique_ptr<SmtpClient> smtp(new SmtpClient(this->smtphost, 25, SmtpClient::TcpConnection));

    if (this->smtpauth) {
        smtp->setUser(this->smtpuser);
        smtp->setPassword(this->smtppsw);
    }

    std::unique_ptr<MimeMessage> message(new MimeMessage());
    std::unique_ptr<EmailAddress> sender(new EmailAddress(this->mailfromaddr, this->mailfromname));
    message->setSender(&*sender);
    message->addRecipient(&receiver);
    message->setSubject(subject);
    std::unique_ptr<MimeText> text(new MimeText());
    text->setText(txt);
    message->addPart(&*text);
    if(!smtp->connectToHost())
        return false;

    if (this->smtpauth) {
        smtp->login();
    }

    if(!smtp->sendMail(*message))
        return false;
    smtp->quit();
    return true;
}

bool mailer::sendConfirmEmail(std::string &uid, frmIscrizione &frm) {
	EmailAddress receiver(QString::fromStdString(frm.email.value()), QString::fromStdString(frm.name.value()));

    QHash<QString, QString> repl;
    repl.insert("uid", QString::fromStdString(uid));
    repl.insert("name", QString::fromStdString(frm.name.value()));
    std::shared_ptr<QString> txt = ReplaceValues(this->confirm_registration_text, &repl);

    return this->send(*txt, this->confirm_registration_subject, receiver);
}

bool mailer::sendPasswordRecoveryEmail(std::string &uid, frmPasswordRecoveryInit &frm) {
	EmailAddress receiver(QString::fromStdString(frm.email.value()));

    QHash<QString, QString> repl;
    repl.insert("email", QString::fromStdString(frm.email.value()));
    repl.insert("url", QString::fromStdString(this->password_recovery_url + "email=" + frm.email.value() + "&token=" + uid));
    std::shared_ptr<QString> txt = ReplaceValues(this->password_recovery_text, &repl);

    return this->send(*txt, this->password_recovery_subject, receiver);
}

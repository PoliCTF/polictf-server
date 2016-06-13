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
    smtphost(), smtpuser(), smtppsw(), mailfromaddr(), mailfromname(), subject(), text() {
    cppcms::json::value settings = srv->settings();
    this->smtphost = QString::fromStdString(settings.get<std::string>("smtp.host"));
    this->smtpuser = QString::fromStdString(settings.get<std::string>("smtp.user"));
    this->smtppsw = QString::fromStdString(settings.get<std::string>("smtp.password"));

    if (settings.get<int>("smtp.auth") >= 1) {
        this->smtpauth = true;
    } else {
        this->smtpauth = false;
    }

    this->subject = QString::fromStdString(settings.get<std::string>("mail.subject"));
    this->text = QString::fromStdString(settings.get<std::string>("mail.text"));
    this->mailfromaddr = QString::fromStdString(settings.get<std::string>("mail.from_address"));
    this->mailfromname = QString::fromStdString(settings.get<std::string>("mail.from_name"));
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

void mailer::sendEmail(std::string &uid, frmIscrizione &frm) {
    std::unique_ptr<SmtpClient> smtp(new SmtpClient(this->smtphost, 25, SmtpClient::TcpConnection));

    if (this->smtpauth) {
        smtp->setUser(this->smtpuser);
        smtp->setPassword(this->smtppsw);
    }

    std::unique_ptr<MimeMessage> message(new MimeMessage());
    std::unique_ptr<EmailAddress> sender(new EmailAddress(this->mailfromaddr, this->mailfromname));
    std::unique_ptr<EmailAddress> receiver(new EmailAddress(QString::fromStdString(frm.email.value()), QString::fromStdString(frm.name.value())));
    message->setSender(&*sender);
    message->addRecipient(&*receiver);
    message->setSubject(this->subject);
    std::unique_ptr<MimeText> text(new MimeText());
    QHash<QString, QString> repl;
    repl.insert("uid", QString::fromStdString(uid));
    repl.insert("name", QString::fromStdString(frm.name.value()));
    std::shared_ptr<QString> txt = ReplaceValues(this->text, &repl);
    text->setText(*txt);
    message->addPart(&*text);
    smtp->connectToHost();

    if (this->smtpauth) {
        smtp->login();
    }

    smtp->sendMail(*message);
    smtp->quit();

}

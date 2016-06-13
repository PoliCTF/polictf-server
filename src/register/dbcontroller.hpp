#ifndef REGISTER_DBCONTROLLER_H
#define REGISTER_DBCONTROLLER_H

#include "frmIscrizione.hpp"

#include <cppcms/json.h>
#include <cppcms/http_context.h>

#include <QtSql>

#include <memory>
#include <unordered_map>
#include <mutex>
#include <thread>


class dbcontroller {
public:
    static std::shared_ptr<dbcontroller> getInstance();

    ~dbcontroller();

    bool checkDuplicates(std::string name, std::string email);
    bool insert(frmIscrizione *frm, std::string *uid);
    bool confirm(std::string *code);
    bool isSpammer();

protected:
    dbcontroller();

    bool handleError(std::shared_ptr<QSqlQuery> q);

    QSqlDatabase db;

    static std::unordered_map<std::thread::id,  std::shared_ptr<dbcontroller>> thread_instances;
    static std::mutex instances_mutex;
};

#endif // DBCONTROLLER_H

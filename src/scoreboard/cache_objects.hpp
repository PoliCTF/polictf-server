#ifndef SCOREBOARD_CACHE_OBJECTS_H
#define SCOREBOARD_CACHE_OBJECTS_H

#include <cppcms/archive_traits.h>
#include <cppcms/serialization.h>

#include <unordered_map>
#include <vector>

struct team : public cppcms::serializable {
    int id;
    short valid;
    void serialize(cppcms::archive &a) {
        a & id & valid;
    }
};

struct challengesol: public cppcms::serializable {
    int points;
    int id;
    short valid;
    void serialize(cppcms::archive &a) {
        a & id & valid & points;
    }
};

struct teamip : public cppcms::serializable {
    int id;
    std::vector<std::string> seenip;
    void serialize(cppcms::archive &a) {
        a & id & seenip;
    }
};

struct teamSolution : public cppcms::serializable {
    int id;

    int solutionPoints;
    int flashChallenge;
    void serialize(cppcms::archive &a) {
        a & id & flashChallenge & solutionPoints;
    }
};

struct teamAlert : public cppcms::serializable {
    std::string message;
    int timestamp;
    int points;
    void serialize(cppcms::archive &a) {
        a & message & timestamp & points;
    }
};

struct teamSolutionList : public cppcms::serializable {
    //Yo dawg, i heard you like memfuckups
    std::vector<teamSolution> challenges;

    void serialize(cppcms::archive &a) {
        a & challenges;
    }
};

struct teamAlertList : public cppcms::serializable {
    //Yo dawg, i heard you like memfuckups
    std::vector<teamAlert> alerts;

    void serialize(cppcms::archive &a) {
        a & alerts;
    }
};

struct teamInfo : public cppcms::serializable {
    unsigned int points; //????
    std::string name;

    void serialize(cppcms::archive &a) {
        a & points & name;
    }
};

#endif //SCOREBOARD_CACHE_OBJECTS_H

#ifndef SCOREBOARD_VERIFIER_H
#define SCOREBOARD_VERIFIER_H

#include "cache_objects.hpp"
#include <string>

class verifier {
public:
    struct response {
        int points;
        int challengeid;
    };

    response verify(const std::string &sol);

    const static int TOO_FAST = -4;
    const static int CANNOTSAVE = -3;
    const static int ALREADY_SOLVED = -2;
    const static int WRONG = -1;
};

#endif // SCOREBOARD_VERIFIER_H

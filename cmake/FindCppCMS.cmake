message("-- Looking for CPPCMS")

option(CPPCMS_PATH "Path to CPPCMS installation" "/usr")

find_library(CPPCMS_LIBRARY cppcms ${CPPCMS_PATH} ${CPPCMS_PATH}/lib)
find_path(CPPCMS_INCLUDE_DIR cppcms/http_context.h ${CPPCMS_PATH}/include)

if((${CPPCMS_LIBRARY} MATCHES "NOTFOUND") OR (${CPPCMS_INCLUDE_DIR} MATCHES "NOTFOUND"))
    message(FATAL_ERROR "Cannot find CPPCMS")
else()
    message("-- CPPCMS Found")
endif()

message("-- Looking for CPPCMS's Booster")

option(BOOSTER_PATH "Path to CPPCMS's Booster installation" ${CPPCMS_PATH})

find_library(CPPCMSBOOSTER_LIBRARY booster ${BOOSTER_PATH} ${BOOSTER_PATH}/lib)
find_path(CPPCMSBOOSTER_INCLUDE_DIR booster/regex.h ${BOOSTER_PATH}/include)

if((${CPPCMSBOOSTER_LIBRARY} MATCHES "NOTFOUND") OR (${CPPCMSBOOSTER_INCLUDE_DIR} MATCHES "NOTFOUND"))
    message(FATAL_ERROR "Cannot find CPPCMS's Booster")
else()
    message("-- CPPCMS's Booster Found")
endif()


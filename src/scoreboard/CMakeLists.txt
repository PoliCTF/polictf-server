find_package(Qt4 4.6 REQUIRED QtCore QtSql QtNetwork)
find_package(CppCMS 1.0.0 REQUIRED)

set(SCOREBOARD_SRCS
    verifier.cpp
    main.cpp
    dbcontroller.cpp
    teamstatus.cpp
)

include_directories(
    ${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTSQL_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CPPCMSBOOSTER_INCLUDE_DIR} ${CPPCMS_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
)

add_executable(scoreboard ${SCOREBOARD_SRCS})
set_target_properties(scoreboard PROPERTIES AUTOMOC TRUE)

target_compile_options(scoreboard PRIVATE
    -O2 -march=native -g -std=c++0x
    ${SECURE_COMPILER_FLAGS}
)

target_link_libraries(scoreboard
    ${QT_QTCORE_LIBRARY} ${QT_QTSQL_LIBRARY} ${QT_QTNETWORK_LIBRARY}
    ${CPPCMSBOOSTER_LIBRARY} ${CPPCMS_LIBRARY}
    ${SECURE_LINKER_FLAGS}
)

install(TARGETS scoreboard DESTINATION bin/)

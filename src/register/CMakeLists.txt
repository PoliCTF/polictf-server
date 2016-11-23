find_package(Qt4 4.6 REQUIRED QtCore QtSql QtNetwork)
find_package(CppCMS REQUIRED)
find_package(MySQL REQUIRED)

add_subdirectory(smtp-qt)

set(REGISTRATION_SRCS
    main.cpp
    frmIscrizione.cpp
    frmPasswordRecovery.cpp
    frmPasswordRecoveryInit.cpp
    dbcontroller.cpp
    mailer.cpp
)

include_directories(
    ${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTSQL_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CPPCMSBOOSTER_INCLUDE_DIR} ${CPPCMS_INCLUDE_DIR}
)

add_executable(registration ${REGISTRATION_SRCS})
set_target_properties(registration PROPERTIES AUTOMOC TRUE)

target_compile_options(registration PRIVATE
    -O2 -march=native -g -std=c++0x
    ${SECURE_COMPILER_FLAGS}
)

target_link_libraries(registration
    smtp
    ${QT_QTCORE_LIBRARY} ${QT_QTSQL_LIBRARY} ${QT_QTNETWORK_LIBRARY}
    ${CPPCMSBOOSTER_LIBRARY} ${CPPCMS_LIBRARY}
    ${SECURE_LINKER_FLAGS}
)

install(TARGETS registration DESTINATION bin/)

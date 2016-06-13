find_package(Qt4 4.6 REQUIRED QtCore QtNetwork)

set(SMTP_SRCS
    emailaddress.cpp
    mimehtml.cpp
    mimepart.cpp
    mimeattachment.cpp
    mimeinlinefile.cpp
    mimetext.cpp
    mimecontentformatter.cpp
    mimemessage.cpp
    quotedprintable.cpp
    mimefile.cpp
    mimemultipart.cpp
    smtpclient.cpp
)

include_directories(
    ${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTNETWORK_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(smtp ${SMTP_SRCS})
set_target_properties(smtp PROPERTIES AUTOMOC TRUE)

target_compile_options(smtp PRIVATE
    -O2 -march=native -g -std=c++0x
    ${SECURE_COMPILER_FLAGS}
)

target_link_libraries(smtp
    ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY}
    ${SECURE_LINKER_FLAGS}
)

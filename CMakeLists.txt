project(polictf-server)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(SECURE_COMPILER_FLAGS
    -Wall -Wextra -Wpedantic
    -Wno-deprecated-register
    #-Wconversion -Wsign-conversion
    -Wformat-security
    -Werror
    -fstack-protector-all -Wstack-protector --param ssp-buffer-size=4
    -fPIE
    -ftrapv
    -D_FORTIFY_SOURCE=2
)

set(SECURE_LINKER_FLAGS
    -Wl,-z,relro,-z,now
)

add_subdirectory(src/register register)
add_subdirectory(src/scoreboard scoreboard)

cmake_minimum_required(VERSION 3.7...3.19)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(gndb-c  VERSION 0.1
    DESCRIPTION "A graph native database written in C"
    LANGUAGES C)

# Set Compiler
set(CMAKE_C_COMPILER "clang")
message("-- Compiler: " ${CMAKE_C_COMPILER})

# Compiler flags and related options
set(CMAKE_C_STANDARD 11)
set(__STDC_LIB_EXT1__)
set(__STDC_WANT_LIB_EXT1__ 1)
# -fsanitize=address,undefined
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -fsanitize=address,undefined -Wall -Wextra -Werror -Wsequence-point -Wcast-align -Wstrict-prototypes -Wstrict-aliasing -fstack-protector -Wformat -Wformat-security -Werror=format-security -ferror-limit=0 -ftest-coverage -fprofile-arcs")
set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -ffast-math -fPIC -march=native -mtune=native -m64 -Ofast")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(ProcessorCount)
ProcessorCount(N)
add_compile_definitions(THREADS=${N}/2)

if (CMAKE_BUILD_TYPE MATCHES DEBUG)
    message("-- Flags: ${CMAKE_C_FLAGS_DEBUG}")
elseif(CMAKE_BUILD_TYPE MATCHES RELEASE)
    message("-- Flags: ${CMAKE_C_FLAGS_RELEASE}")
endif()

# Utilities
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message("-- Using CCache")
endif()

if (CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(FORTIFY_SOURCE 2)
        set(CMAKE_C_CLANG_TIDY
             clang-tidy;
                -header-filter=.*;
                -checks=-*,bugprone-*,clang-analyzer-*,linuxkernel-*,misc*,performance-inefficient-*,readability-*,-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling,-bugprone-macro-parentheses,-bugprone-reserved-identifier)
     message("-- Using Clang-Tidy")

    set(CMAKE_C_INCLUDE_WHAT_YOU_USE include-what-you-use)
    set(LINK_WHAT_YOU_USE)

    message("-- Using Include-What-You-Use")
    set(CMAKE_C_CPPCHECK "cppcheck")
endif()

# Subdirectories to consider

message("-- Adding doc/")
add_subdirectory(doc/)
message("-- Adding src/")
add_subdirectory(src/)

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    include(CTest)
    message("-- adding tests")
    add_subdirectory(test/)
endif()


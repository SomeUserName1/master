cmake_minimum_required(VERSION 3.7...3.19)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(gndb-c  VERSION 0.1
                DESCRIPTION "A graph native database written in C"
                LANGUAGES C)

# Set Compiler
set(CMAKE_C_COMPILER "clang")
message("Compiler:" ${CMAKE_C_COMPILER})

# Compiler flags and related options
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=undefined,address
    -Wall -Wpedantic -Wextra -Weffc++ -Werror -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fPIC -march=native -m64
    -O3")

include(ProcessorCount)
ProcessorCount(N)
add_compile_definitions(THREADS=${N}/2)

if (CMAKE_BUILD_TYPE EQUAL DEBUG)
    message("Flags: ${CMAKE_C_FLAGS_DEBUG}")
elseif(CMAKE_BUILD_TYPE EQUAL RELEASE)
    message("Flags: ${CMAKE_C_FLAGS_RELEASE}")
endif()

# Utilities
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message("Using CCache")
endif()

if(CMAKE_BUILD_TYPE EQUAL DEBUG)
    set(CMAKE_C_CLANG_TIDY
            clang-tidy;
                -header-filter=.;
                -checks=*;
                -warnings-as-errors=*;)
    message("Using Clang-Tidy")
    set(CMAKE_C_INCLUDE_WHAT_YOU_USE include-what-you-use)
    set(LINK_WHAT_YOU_USE)
    message("Using Include-What-You-Use")
    set(CMAKE_C_CPPCHECK "cppcheck --enable=all")
endif()

# Subdirectories to consider
add_subdirectory(doc/)
add_subdirectory(src/)

if(CMAKE_BUILD_TYPE EQUAL DEBUG)
    add_subdirectory(tests/)
endif()


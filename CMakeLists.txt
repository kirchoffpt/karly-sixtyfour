cmake_minimum_required(VERSION 3.15)
project(karly64 VERSION 1.1.3)

# Use C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Prefer pthread on UNIX
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

message("Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")

if(NOT CMAKE_CONFIGURATION_TYPES)
    message("No config types set, switching build to Release.")
    message("To build Debug in single-config build, pass -DCMAKE_BUILD_TYPE=Debug to cmake.")
    set(CMAKE_BUILD_TYPE Release)
endif()

message("Build type: ${CMAKE_BUILD_TYPE}")

############################################################
# Modern GoogleTest fetch using FetchContent
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

# Make googletest available in this build
FetchContent_MakeAvailable(googletest)

############################################################
# Compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Oi /Ob2 /Ot /GL /EHsc")
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG")
endif()

if(UNIX)
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -funroll-loops -march=native -mtune=native -flto")
endif()

############################################################
# Add your source directories
add_subdirectory(src)

enable_testing()
add_subdirectory(test)
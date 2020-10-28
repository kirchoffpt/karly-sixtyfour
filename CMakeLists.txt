cmake_minimum_required(VERSION 3.13)
project(karly64)
set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

message("Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")

if(NOT CMAKE_CONFIGURATION_TYPES)
    message("No config types set, switching build to release.")
    message("To build debug in single configuration build use the below argument in cmake call.")
    message(-DCMAKE_CONFIGURATION_TYPES=Debug)
    set(CMAKE_BUILD_TYPE Release)
endif()

message("Build type: ${CMAKE_BUILD_TYPE}")

if(MSVC)
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Oi /Ob2 /Ot /GL /EHsc")
    set(CMAKE_EXE_LINKER_FLAGS "")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG")
endif(MSVC)

if(UNIX)
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "-Wall")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -funroll-loops -march=native -mtune=native -flto")
endif(UNIX)	

add_subdirectory(src)
add_subdirectory(test)

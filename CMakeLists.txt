cmake_minimum_required(VERSION 3.13)
project(karly64)
set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


if(MSVC)
    set(CMAKE_CXX_FLAGS "/O2 /Oi /Ob2 /Ot /GL /EHsc")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Oi /Ob2 /Ot /GL /EHsc")
endif(MSVC)

if(UNIX)
    set(CMAKE_CXX_FLAGS "-O3 -funroll-loops -march=native -mtune=native -flto -Wall" )
    set(CMAKE_CXX_FLAGS_DEBUG "-Wall")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -funroll-loops -march=native -mtune=native -flto")
endif(UNIX)

add_subdirectory(src)
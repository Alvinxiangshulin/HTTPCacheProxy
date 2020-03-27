cmake_minimum_required(VERSION 3.0)
project(proxy)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)
find_package(Threads)

add_executable(proxy main.cpp utils.cpp response.cpp request.cpp proxyexceptions.cpp proxyexceptions.h logger.cpp cacheitem.cpp cacheitem.h cache.cpp cache.h)

target_link_libraries(proxy ${CMAKE_THREAD_LIBS_INIT} )

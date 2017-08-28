cmake_minimum_required(VERSION 3.3)
project(xlrest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    src/Config.cpp
    src/Config.h
    src/Declarations.h
    src/dllmain.cpp
    src/HttpServer.cpp
    src/HttpServer.h
    src/JsonObject.cpp
    src/JsonObject.h
    src/Log.cpp
    src/Log.h
    src/Operation.cpp
    src/Operation.h
    src/Parameter.cpp
    src/Parameter.h
    src/Server.cpp
    src/Server.h
    src/Stat.cpp
    src/Stat.h
    src/stdafx.cpp
    src/stdafx.h
    src/targetver.h
    src/Timeout.cpp
    src/Timeout.h
    src/xlcall.cpp
    src/xlcall.h
    src/XlOperTable.cpp
    src/XlOperTable.h
    test/stdafx.cpp
    test/stdafx.h
    test/targetver.h
    test/unittest1.cpp)

add_executable(xlrest ${SOURCE_FILES})
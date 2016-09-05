cmake_minimum_required(VERSION 3.5)
project(auth)

include_directories("/usr/local/opt/openssl/include")
include_directories("/usr/local/opt/protobuf/include")
include_directories("/usr/local/include")
include_directories("../framework/")
include_directories("../../PT_Proto/compiled")



set(CMAKE_LIBRARY_PATH "${CMAKE_LIBRARY_PATH}:/usr/local/opt/openssl/lib:/usr/local/opt/protobuf/lib:/usr/local/lib")
set(CMAKE_LIBRARY_PATH "${CMAKE_LIBRARY_PATH}:../framework/out")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    db_ext.cpp
    db_ext.hpp
    main.cpp
    service.cpp
    service.hpp)

add_executable(auth ${SOURCE_FILES})

target_link_libraries(auth framework protobuf crypto)
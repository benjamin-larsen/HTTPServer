cmake_minimum_required(VERSION 4.1)
project(HTTPServer)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(includes/boost)

add_executable(HTTPServer main.cpp)

target_link_libraries(HTTPServer PRIVATE Boost::asio)

if(WIN32)
    target_link_libraries(HTTPServer PRIVATE ws2_32)
endif()
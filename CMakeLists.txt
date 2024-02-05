cmake_minimum_required(VERSION 3.27)

project(HelloWorld)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(
    HelloWorld
    main.cc
)
cmake_minimum_required(VERSION 3.27)

project(HelloWorld)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(
  tests
  test/chain.cpp
  test/observable.cpp
  test/maybe.cpp
  test/task.cpp
  test/effect.cpp
  test/single-execution.cpp
  test/reader.cpp
)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

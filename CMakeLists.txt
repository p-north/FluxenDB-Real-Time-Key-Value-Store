cmake_minimum_required(VERSION 3.14)

project(FLUXENDB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

# Collect source files
file(GLOB_RECURSE SERVER_SRC
    "src/server/*.cpp"
    "src/utils/*.cpp"
)
list(FILTER SERVER_SRC EXCLUDE REGEX ".*/main\\.cpp$")

file(GLOB_RECURSE CLIENT_SRC
    "src/client/*.cpp"
)
list(FILTER CLIENT_SRC EXCLUDE REGEX ".*/main\\.cpp$")

#creating server library
add_library(server_lib
    ${SERVER_SRC}
)
# Server executable
add_executable(server src/server/main.cpp)

target_link_libraries(server PRIVATE server_lib Threads::Threads)

#creating client library
add_library(client_lib
    ${CLIENT_SRC}
)
# Client executable
add_executable(client src/client/main.cpp)

target_link_libraries(client PRIVATE client_lib server_lib Threads::Threads)


# Tests
file(GLOB_RECURSE TEST_SRC
    "tests/*.cpp"
)

add_executable(tests ${TEST_SRC})

target_link_libraries(tests
    PRIVATE
    server_lib
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(tests)
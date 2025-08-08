cmake_minimum_required(VERSION 3.13)

project(redis-starter-cpp)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Threads REQUIRED)
# find_package(asio CONFIG REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/src)
add_executable(server ${SOURCE_FILES})

# target_link_libraries(server PRIVATE asio asio::asio)
target_link_libraries(server PRIVATE Threads::Threads)

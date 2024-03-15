cmake_minimum_required(VERSION 3.20)
project(TCP_Client)

# init options flags
option(NON_BLOCKING "enable non-blocking sockets (or no)" OFF)


# check NON_BLOCKING flag and add definition
message("NON_BLOCKING: ${NON_BLOCKING}")
if (NON_BLOCKING)
    message("NON_BLOCKING enabled!!")
    add_definitions(-D NON_BLOCKING)
endif()

message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Debug enabled!!")
    add_definitions(-D DEBUG)
endif()


# set compiler flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# set include directories
include_directories(
        "${PROJECT_SOURCE_DIR}/headers"
        "${PROJECT_SOURCE_DIR}/headers/interfaces/"
        "${PROJECT_SOURCE_DIR}/tpp"
)

# set source files
file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/tpp/*.tpp"
)

# add executable files
add_executable(${PROJECT_NAME} main.cpp ${all_SRCS})
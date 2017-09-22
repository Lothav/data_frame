cmake_minimum_required(VERSION 3.7)
project(data_frame)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES HandleErrors.h)
set(HANDLE_ERRORS_FILES main.cpp)

add_executable(data_frame ${SOURCE_FILES} ${HANDLE_ERRORS_FILES})
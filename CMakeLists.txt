cmake_minimum_required(VERSION 3.7)
project(data_frame)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp HandleErrors.h Sender.h Module.h Receiver.h)

add_executable(data_frame ${SOURCE_FILES})
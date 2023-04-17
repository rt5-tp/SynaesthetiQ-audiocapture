cmake_minimum_required(VERSION 3.0.0)

project(test)

set(SOURCES TestPingPongBuffer.cpp ../src/AudioCapture/PingPongBuffer.cpp)

add_executable(test ${SOURCES})

target_link_libraries(test gtest pthread)
cmake_minimum_required(VERSION 3.0)
project(SynaesthetiQ-AudioCapture)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fsanitize=address -fno-omit-frame-pointer")

set(SOURCES main.cpp AudioCapture/PingPongBuffer.cpp AudioCapture/AudioCapture.cpp GenreClassification/GenreClassifier.cpp FFTProcessor.cpp)

include_directories(AudioCapture)
include_directories(GenreClassification)


add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} asound SDL2 stdc++ pthread fftw3)


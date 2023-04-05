cmake_minimum_required(VERSION 3.0)
project(SynaesthetiQ-AudioCapture)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fsanitize=address -fno-omit-frame-pointer")

set(SOURCES main.cpp AudioCapture/PingPongBuffer.cpp AudioCapture/AudioCapture.cpp GenreClassification/GenreClassifier.cpp FFTProcessor.cpp)
set(LOWRESFFT_SOURCES lowresfft_test.cpp AudioCapture/PingPongBuffer.cpp AudioCapture/AudioCapture.cpp GenreClassification/GenreClassifier.cpp FFTProcessor.cpp)

include_directories(AudioCapture)
include_directories(GenreClassification)
include_directories(GenreClassification/cpp_genre_classifier)

find_package( CURL REQUIRED )

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} asound SDL2 stdc++ pthread fftw3 CURL::libcurl)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}.exe")

add_executable(lowresfft ${LOWRESFFT_SOURCES})
target_link_libraries(lowresfft asound SDL2 stdc++ pthread fftw3 CURL::libcurl)
set_target_properties(lowresfft PROPERTIES OUTPUT_NAME "lowresfft.exe")



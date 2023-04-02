cmake_minimum_required(VERSION 3.18.4)

project(GenreClassifier)

set(SOURCES main.cpp)

add_executable(GenreClassifier ${SOURCES})

find_package( CURL REQUIRED )

target_link_libraries(GenreClassifier CURL::libcurl)

#set(HEADERS synaesthetiq.hpp Colour.hpp)

target_include_directories(GenreClassifier PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
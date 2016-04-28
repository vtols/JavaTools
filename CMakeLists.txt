cmake_minimum_required(VERSION 2.8)

project(JavaTools)

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

set(BUILD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(BUILD_CONFIG_PATH ${BUILD_PATH}/config)

set(SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories ("${INCLUDE_PATH}")

## For config.h
include_directories ("${BUILD_CONFIG_PATH}")

set(BINARY javac)

set(SOURCES
    ${SOURCE_PATH}/main.cc
    ${SOURCE_PATH}/char_stream.cc
    ${SOURCE_PATH}/file_stream.cc
    ${SOURCE_PATH}/java_lexer.cc)

set(HEADERS
    ${INCLUDE_PATH}/char_stream.h
    ${INCLUDE_PATH}/file_stream.h
    ${INCLUDE_PATH}/java_lexer.h)

add_executable(${BINARY} ${SOURCES})

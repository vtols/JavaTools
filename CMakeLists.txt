cmake_minimum_required(VERSION 2.8)

project(JavaTools)

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
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
    ${SOURCE_PATH}/java_lexer.cc
    ${SOURCE_PATH}/java_unit.cc
    ${SOURCE_PATH}/java_parser.cc)

set(HEADERS
    ${INCLUDE_PATH}/char_stream.h
    ${INCLUDE_PATH}/file_stream.h
    ${INCLUDE_PATH}/byte_stream.h
    ${INCLUDE_PATH}/file_byte_stream.h
    ${INCLUDE_PATH}/java_lexer.h
    ${INCLUDE_PATH}/java_unit.h
    ${INCLUDE_PATH}/java_parser.h)

add_executable(${BINARY} ${SOURCES})

set(DUMP_BINARY dump)

set(DUMP_SOURCES
    ${SOURCE_PATH}/dump.cc
    ${SOURCE_PATH}/byte_stream.cc
    ${SOURCE_PATH}/byte_stream_writer.cc
    ${SOURCE_PATH}/file_byte_stream.cc
    ${SOURCE_PATH}/file_byte_stream_writer.cc
    ${SOURCE_PATH}/java_class.cc)

set(DUMP_HEADERS
    ${INCLUDE_PATH}/byte_stream.cc
    ${INCLUDE_PATH}/byte_stream_writer.cc
    ${INCLUDE_PATH}/file_byte_stream.cc
    ${INCLUDE_PATH}/file_byte_stream_writer.cc
    ${INCLUDE_PATH}/java_class.cc)

add_executable(${DUMP_BINARY} ${DUMP_SOURCES})

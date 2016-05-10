cmake_minimum_required(VERSION 2.8)

project(JavaTools)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

set(BUILD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

include_directories (
    "${PROJECT_BINARY_DIR}"
    "${INCLUDE_PATH}"
)

set(LIB_javatools javatools)
set(SOURCES_javatools

    ${SOURCE_PATH}/javac.cc
    ${SOURCE_PATH}/io/char_stream.cc
    ${SOURCE_PATH}/io/file_stream.cc
    ${SOURCE_PATH}/io/byte_reader.cc
    ${SOURCE_PATH}/io/byte_writer.cc
    ${SOURCE_PATH}/io/file_byte_reader.cc
    ${SOURCE_PATH}/io/file_byte_writer.cc
    ${SOURCE_PATH}/io/byte_buffer.cc
    ${SOURCE_PATH}/parser/java_lexer.cc
    ${SOURCE_PATH}/parser/java_unit.cc
    ${SOURCE_PATH}/parser/java_parser.cc
    ${SOURCE_PATH}/class/java_class_builder.cc
    ${SOURCE_PATH}/class/java_class.cc
)
ADD_LIBRARY(${LIB_javatools} STATIC ${SOURCES_javatools} )

set(BINARY_javac javac)
set(SOURCES_javac

    ${SOURCE_PATH}/javac.cc
    ${SOURCE_PATH}/gen/java_gen.cc
    ${SOURCE_PATH}/gen/java_env.cc
)
add_executable(${BINARY_javac} ${SOURCES_javac})
target_link_libraries(${BINARY_javac} ${LIB_javatools})

set(BINARY_dump dump)
set(SOURCES_dump

    ${SOURCE_PATH}/dump.cc
)
add_executable(${BINARY_dump} ${SOURCES_dump})
target_link_libraries(${BINARY_dump} ${LIB_javatools})

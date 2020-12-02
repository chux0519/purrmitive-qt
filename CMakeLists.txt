cmake_minimum_required(VERSION 3.1.0)

project(purrmitive)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set (CMAKE_CXX_STANDARD 11)


if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()


SET(PURR_BINDING_ROOT "${CMAKE_SOURCE_DIR}/deps/purr-binding")
SET(PURR_BINDING_LIB_PATH "${CMAKE_SOURCE_DIR}/deps/purr-binding/target/release")
link_directories(${PURR_BINDING_LIB_PATH})
if(NOT EXISTS ${PURR_BINDING_LIB_PATH})
      execute_process(COMMAND cargo build --release WORKING_DIRECTORY ${PURR_BINDING_ROOT})
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)

include_directories(src deps)
file(GLOB SOURCES src/*.cc)

add_executable(purrmitive
    ${SOURCES}
)

target_link_libraries(purrmitive Qt5::Widgets purrmitive_rust)

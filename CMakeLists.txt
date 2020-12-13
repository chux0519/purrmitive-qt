cmake_minimum_required(VERSION 3.1.0)

project(purrmitive)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set (CMAKE_CXX_STANDARD 11)


if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()


SET(PURR_ROOT "${CMAKE_SOURCE_DIR}/deps/purr")
SET(PURR_LIB_PATH "${CMAKE_SOURCE_DIR}/deps/purr/target/release")
SET(PURR_LIB_NAME purrmitive_ffi)
link_directories(${PURR_LIB_PATH})
if(NOT EXISTS ${PURR_LIB_PATH})
      execute_process(COMMAND cargo build --release WORKING_DIRECTORY ${PURR_ROOT})
endif()

find_package(Qt5 COMPONENTS Widgets Svg REQUIRED)

include_directories(src deps)
file(GLOB SOURCES src/*.cc src/rust_bridge.h)

add_executable(purrmitive
    ${SOURCES}
)

target_link_libraries(purrmitive Qt5::Widgets Qt5::Svg ${PURR_LIB_NAME})

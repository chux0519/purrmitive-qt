cmake_minimum_required(VERSION 3.1.0)

project(purrmitive)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)

include_directories(src)
file(GLOB SOURCES src/*.cc)

add_executable(purrmitive
    ${SOURCES}
)

target_link_libraries(purrmitive Qt5::Widgets)

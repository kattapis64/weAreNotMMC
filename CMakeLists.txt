cmake_minimum_required(VERSION 3.23)
project(MMC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(MMC main.cpp
        CSC.cpp
        ovh.cpp)

find_package(HIGHS REQUIRED)

# Link the target first
target_link_libraries(MMC PRIVATE highs::highs)

# Ensure include paths apply directly to the executable
target_include_directories(MMC PRIVATE
        /usr/local/include
        /usr/local/include/highs
)
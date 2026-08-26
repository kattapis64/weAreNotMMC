cmake_minimum_required(VERSION 3.23)
project(MMC LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(MMC main.cpp)

find_package(HIGHS REQUIRED)
target_link_libraries(MMC PRIVATE highs::highs)
target_include_directories(MMC PRIVATE 
    /usr/local/include/highs
    /usr/local/include/highs/lp_data # explicitly add if custom installed
)


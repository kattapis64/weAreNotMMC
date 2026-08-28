cmake_minimum_required(VERSION 3.23)
project(MMC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
add_compile_options(-O3)

add_executable(MMC main.cpp
        CSC.cpp
        ovh.cpp)

# --- OpenMP ---------------------------------------------------------------
# Do NOT just add_compile_options(-fopenmp): that only affects compilation,
# not linking, so the compiler emits calls into the OpenMP runtime
# (__kmpc_fork_call etc.) but nothing tells the linker to pull in libomp.
# find_package(OpenMP) + OpenMP::OpenMP_CXX handles both the compile flag
# AND the correct runtime library/link flags for whichever compiler CMake
# is using (libomp for clang, libgomp for gcc).
find_package(OpenMP REQUIRED)
target_link_libraries(MMC PRIVATE OpenMP::OpenMP_CXX)

# --- HiGHS ------------------------------------------------------------------
find_package(HIGHS REQUIRED)
target_link_libraries(MMC PRIVATE highs::highs)

target_include_directories(MMC PRIVATE
        /usr/local/include
        /usr/local/include/highs
)
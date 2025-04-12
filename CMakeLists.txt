cmake_minimum_required(VERSION 3.10)
project(flexa_interpreter)

# basic config
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# source files
file(GLOB SOURCES "src/*.cpp")

# main exe
add_executable(flexa_interpreter 
    ${SOURCES}
)

# include src/ for headers
target_include_directories(flexa_interpreter PRIVATE src/)

# opt: additional flags (debug, optmization, etc.)
target_compile_options(flexa_interpreter PRIVATE -Wall -Wextra)

# opt: link external libs (ex: OpenGL, SDL, etc.)
# target_link_libraries(flexa_interpreter PRIVATE SDL2 OpenGL)

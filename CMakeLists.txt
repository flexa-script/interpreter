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

# flags de compilação (avisos e afins)
target_compile_options(flexa_interpreter PRIVATE -Wall -Wextra)

# flags de linkagem para maior portabilidade
target_link_options(flexa_interpreter PRIVATE -static)

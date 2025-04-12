### **`CMakeLists.txt`** (in project root)
```cmake
cmake_minimum_required(VERSION 3.10)
project(flexa_interpreter)

# Basic config
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define os arquivos fonte (todos os .cpp em src/)
file(GLOB SOURCES "src/*.cpp")

# Cria o executável principal
add_executable(flexa_interpreter 
    ${SOURCES}
)

# Includes src/ dir for headers
target_include_directories(flexa_interpreter PRIVATE src/)

# Optional: Additional flags (debug, optmizations etc.)
target_compile_options(flexa_interpreter PRIVATE -Wall -Wextra)

# Optional: Link external libs (ex: OpenGL, SDL, etc.)
# target_link_libraries(flexa_interpreter PRIVATE SDL2 OpenGL)
```

---

### **How to use:**
1. Debug
```bash
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
./flexa_interpreter
```
2. Release
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
./flexa_interpreter
```

---

### **Directives:**
- **`file(GLOB SOURCES ...)`**: Automatically collect all `.cpp` source files from `src/` dir.
- **`target_include_directories()`**: Adds `src/` to include path for `#include` works.
- **`target_compile_options()`**: Adds flags.
- **`target_link_libraries()`**: Adds external libs.

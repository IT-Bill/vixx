# CMakeLists.txt

cmake_minimum_required(VERSION 3.14)
project(VimLikeEditor)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find ncurses library
find_package(Curses REQUIRED)

# Include directories
include_directories(include)

# Source files
file(GLOB BACKEND_SOURCES "src/backend/*.cpp")
file(GLOB FRONTEND_SOURCES "src/frontend/*.cpp")
set(SOURCES
    ${BACKEND_SOURCES}
    ${FRONTEND_SOURCES}
    src/main.cpp
)

# Add executable
add_executable(vim_like_editor ${SOURCES})

# Link ncurses
target_link_libraries(vim_like_editor PRIVATE ${CURSES_LIBRARIES})

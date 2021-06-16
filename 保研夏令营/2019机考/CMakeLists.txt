cmake_minimum_required(VERSION 3.20)

project(test)

# set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
# set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=undefined,address")
# set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=undefined,address")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

find_library(OpenGL GL REQUIRED)
find_library(SDL2 SDL2 REQUIRED)
find_library(GLEW GLEW REQUIRED)
find_library(pthread pthread REQUIRED)

include_directories(thirdparty)

add_subdirectory(thirdparty/imgui)
add_subdirectory(thirdparty/nanovg)
add_subdirectory(thirdparty/rash)

# add_executable(main main.cpp)
add_executable(linear main.cpp)
add_executable(cuckoo main.cpp)
add_executable(perf perf.cpp)
add_executable(cuckoo-demo cuckoo-demo.cpp)

set(cxx_options
    -Wall
    -Wextra
    -Wno-char-subscripts
    -Wno-sign-compare
)

# target_compile_options(main PRIVATE ${cxx_options})
target_compile_options(linear PRIVATE ${cxx_options} -DLINEAR)
target_compile_options(cuckoo PRIVATE ${cxx_options} -DCUCKOO)
target_compile_options(perf PRIVATE ${cxx_options})

target_link_libraries(cuckoo-demo rash imgui nanovg OpenGL SDL2 GLEW pthread)

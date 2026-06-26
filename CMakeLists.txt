cmake_minimum_required(VERSION 3.28)

project(raw-containers LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CXX_COMPILER_ID:MSVC>:$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")

add_library(raw INTERFACE)

target_include_directories(raw INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_features(raw INTERFACE cxx_std_20)
target_compile_options(raw INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor /W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

enable_testing()

add_subdirectory(examples)
add_subdirectory(tests)

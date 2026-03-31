cmake_minimum_required (VERSION 3.28)

project ("raw-container" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(raw INTERFACE)
target_include_directories(raw INTERFACE include)

add_subdirectory(examples)

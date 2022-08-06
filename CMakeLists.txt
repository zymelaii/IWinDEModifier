cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(IWinDEModifier VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(MSVC)
	add_compile_options(/utf-8 /W2 /MP /Gm-)
endif()

include_directories(src)

add_subdirectory(src)
add_subdirectory(scripts)
cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(IWinDEModifier VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_DEBUG_POSTFIX "d")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/lib)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/lib64)

option(BUILD_DesktopTab "build utils/DesktopTab" ON)
option(BUILD_iwdesh "build utils/iwdesh" ON)
option(BUILD_LazyCraft "build utils/LazyCraft" ON)
option(BUILD_Ethereality "build utils/Ethereality" ON)

if(MSVC)
	add_compile_options(/utf-8 /W2 /MP /Gm-)
else()
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		# add_compile_options(-DNDEBUG -O3)
		add_compile_options(-DNDEBUG)
	elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_compile_options(-DDEBUG -g)
	endif()
endif()

include_directories(src)
add_subdirectory(src)

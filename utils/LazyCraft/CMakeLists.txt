set(UTIL_NAME LazyCraft)

file(GLOB SOURCE_FILES *.cpp)

add_executable(${UTIL_NAME} ${SOURCE_FILES})

target_link_libraries(${UTIL_NAME} PRIVATE imgui-dx11-static)

if(NOT MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		target_link_options(${UTIL_NAME} PRIVATE -Wl,-subsystem,windows)
	endif()
	target_link_options(${UTIL_NAME} PRIVATE --static)
endif()
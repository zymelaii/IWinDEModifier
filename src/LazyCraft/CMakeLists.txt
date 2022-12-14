set(UTIL_NAME LazyCraft)

file(GLOB SOURCE_FILES *.cpp)
file(GLOB UTILS_FILES com/*.cpp widget/*.cpp)
list(APPEND SOURCE_FILES ${UTILS_FILES})

include_directories(.)

add_executable(${UTIL_NAME} ${SOURCE_FILES})
add_dependencies(${UTIL_NAME} lctrap)

target_link_libraries(${UTIL_NAME} PRIVATE iwde-utils imgui-dx11-static)

if(NOT MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		target_link_options(${UTIL_NAME} PRIVATE -Wl,-subsystem,windows)
	endif()
endif()

add_custom_command(TARGET ${UTIL_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory
		${CMAKE_BINARY_DIR}/${UTIL_NAME}
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:${UTIL_NAME}> ${CMAKE_BINARY_DIR}/${UTIL_NAME}
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/${UTIL_NAME}/assets
	COMMAND_EXPAND_LISTS
)

add_subdirectory(trap)
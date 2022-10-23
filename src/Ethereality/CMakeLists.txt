add_executable(Ethereality client.cpp)
target_link_libraries(Ethereality PRIVATE imgui-dx11 iwde-utils)

if(NOT MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		target_link_options(Ethereality PRIVATE -Wl,-subsystem,windows)
	endif()
endif()

add_custom_command(TARGET Ethereality POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory
		${CMAKE_BINARY_DIR}/Ethereality
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:Ethereality> ${CMAKE_BINARY_DIR}/Ethereality
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:imgui-dx11> ${CMAKE_BINARY_DIR}/Ethereality
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/Ethereality/assets
	COMMAND_EXPAND_LISTS
)
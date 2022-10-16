set(UTIL_NAME LazyCraft)

add_library(lctrap MODULE hook.c)

add_custom_command(TARGET lctrap POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory
		${CMAKE_BINARY_DIR}/${UTIL_NAME}/trap
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:lctrap> ${CMAKE_BINARY_DIR}/${UTIL_NAME}/trap/lctrap.dll
	COMMAND_EXPAND_LISTS
)
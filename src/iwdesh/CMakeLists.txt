set(UTIL_NAME iwdesh)
set(PLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/${UTIL_NAME}/plugins)

file(GLOB SOURCE_FILES entry/*.cpp)
file(GLOB_RECURSE HEADER_FILES *.h)

add_library(${UTIL_NAME}-core MODULE ${HEADER_FILES} ${SOURCE_FILES})
target_link_options(${UTIL_NAME}-core PRIVATE --static)
target_link_libraries(${UTIL_NAME}-core PRIVATE iwindeapi-static)

set_target_properties(${UTIL_NAME}-core PROPERTIES PLUGIN_NAME core)

file(GLOB SOURCE_FILES *.cpp)

add_executable(${UTIL_NAME} ${SOURCE_FILES})

target_link_options(${UTIL_NAME} PRIVATE --static)

set_target_properties(${UTIL_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${UTIL_NAME})

add_dependencies(${UTIL_NAME} ${UTIL_NAME}-core)

add_custom_command(TARGET ${UTIL_NAME}-core POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:${UTIL_NAME}-core>
		${PLUGIN_OUTPUT_DIR}/$<TARGET_PROPERTY:${UTIL_NAME}-core,PLUGIN_NAME>.${UTIL_NAME}-plugin
	COMMAND_EXPAND_LISTS
)
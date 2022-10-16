file(GLOB HEADER_FILES iwindeapi/*.h)
file(GLOB SOURCE_FILES iwindeapi/*.cpp)

set(Targets iwindeapi-static iwindeapi)
add_library(iwindeapi-static STATIC ${HEADER_FILES} ${SOURCE_FILES})
add_library(iwindeapi SHARED ${HEADER_FILES} ${SOURCE_FILES})

foreach(target ${Targets})
	target_link_libraries(${target} PRIVATE crypt32 bcrypt)
endforeach()

file(GLOB SOURCE_FILES *.cpp)
add_executable(IWinDEModifier ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(IWinDEModifier PRIVATE iwindeapi imgui-dx11)

if(NOT MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		target_link_options(IWinDEModifier PRIVATE -Wl,-subsystem,windows)
	endif()
	target_link_options(IWinDEModifier PRIVATE --static)
endif()

add_custom_command(TARGET IWinDEModifier POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory
		${CMAKE_BINARY_DIR}/IWinDEModifier
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:IWinDEModifier> ${CMAKE_BINARY_DIR}/IWinDEModifier
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:imgui-dx11> ${CMAKE_BINARY_DIR}/IWinDEModifier
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:iwindeapi> ${CMAKE_BINARY_DIR}/IWinDEModifier
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/IWinDEModifier/assets
	COMMAND_EXPAND_LISTS
)
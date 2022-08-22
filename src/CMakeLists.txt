file(GLOB HEADER_FILES iwindeapi/*.h)
file(GLOB SOURCE_FILES iwindeapi/*.cpp)

add_library(iwindeapi SHARED ${HEADER_FILES} ${SOURCE_FILES})
add_library(iwindeapi-static STATIC ${HEADER_FILES} ${SOURCE_FILES})

file(GLOB SOURCE_FILES *.cpp)
add_executable(IWinDEModifier ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(IWinDEModifier PRIVATE iwindeapi imgui-dx11)

set(Targets iwindeapi iwindeapi-static)
foreach(target ${Targets})
	target_link_libraries(${target} PRIVATE crypt32 bcrypt)
endforeach()

if (NOT MSVC AND CMAKE_BUILD_TYPE STREQUAL "Release")
	target_link_options(IWinDEModifier PRIVATE -Wl,-subsystem,windows)
endif()

add_custom_command(TARGET IWinDEModifier POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  	${CMAKE_SOURCE_DIR}/assets
	${CMAKE_BINARY_DIR}
  COMMAND_EXPAND_LISTS
)
find_package(libuv)
find_package(nlohmann_json)

set(SOURCE_FILES
	PrivateOnlyHub/PrivateOnlyHub.cpp
	PrivateOnlyHub/PrivateMsgFactory.cpp
	PrivateOnlyHub/PrivateMsg.cpp
	client.cpp)

add_executable(Ethereality ${SOURCE_FILES})
target_link_libraries(Ethereality PRIVATE imgui-dx11 iwde-utils d3d11 nlohmann_json::nlohmann_json)

add_executable(Win32Dx11Example win32_dx11_example.cpp)
target_link_libraries(Win32Dx11Example PRIVATE imgui-dx11 d3d11)

add_executable(TcpServer tcp_server.cpp)
target_link_libraries(TcpServer PRIVATE uv_a)

add_executable(TcpClient tcp_client.cpp)
target_link_libraries(TcpClient PRIVATE uv_a)

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
find_package(libuv REQUIRED)
find_package(nlohmann_json REQUIRED)

set(SOURCE_FILES
	PrivateOnlyHub/PrivateOnlyHub.cpp
	PrivateOnlyHub/PrivateMsgFactory.cpp
	PrivateOnlyHub/PrivateMsg.cpp
	PrivateOnlyHub/client.cpp)

add_executable(PrivateOnlyHub ${SOURCE_FILES})
target_link_libraries(PrivateOnlyHub PRIVATE imgui-dx11 iwde-utils d3d11 nlohmann_json::nlohmann_json)

set(SOURCE_FILES
	GroupOnlyHub/GroupOnlyHub.cpp
	GroupOnlyHub/GroupMsgFactory.cpp
	GroupOnlyHub/GroupMsg.cpp
	GroupOnlyHub/client.cpp)

add_executable(GroupOnlyHub ${SOURCE_FILES})
target_link_libraries(GroupOnlyHub PRIVATE imgui-dx11 iwde-utils d3d11 nlohmann_json::nlohmann_json)

set(SOURCE_FILES
	GroupOnlyHub/tcpserver.cpp
	GroupOnlyHub/server.cpp)

add_executable(GroupHubServer ${SOURCE_FILES})
target_link_libraries(GroupHubServer PRIVATE libuv::uv_a nlohmann_json::nlohmann_json)

set(SOURCE_FILES
	GroupOnlyHub/tcpclient.cpp
	GroupOnlyHub/tcpclient.cpp
	GroupOnlyHub/GroupOnlyHub.cpp
	GroupOnlyHub/GroupMsgFactory.cpp
	GroupOnlyHub/GroupMsg.cpp)

add_executable(GroupHubClient ${SOURCE_FILES})
target_link_libraries(GroupHubClient PRIVATE imgui-dx11 d3d11 libuv::uv_a nlohmann_json::nlohmann_json)

add_executable(Win32Dx11Example win32_dx11_example.cpp)
target_link_libraries(Win32Dx11Example PRIVATE imgui-dx11 d3d11)

if(NOT MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		target_link_options(PrivateOnlyHub PRIVATE -Wl,-subsystem,windows)
		target_link_options(GroupOnlyHub PRIVATE -Wl,-subsystem,windows)
	endif()
endif()

add_custom_command(TARGET PrivateOnlyHub POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory
		${CMAKE_BINARY_DIR}/Ethereality
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:PrivateOnlyHub> ${CMAKE_BINARY_DIR}/Ethereality
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:imgui-dx11> ${CMAKE_BINARY_DIR}/Ethereality
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/Ethereality/assets
	COMMAND_EXPAND_LISTS
)

add_custom_command(TARGET GroupOnlyHub POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory
		${CMAKE_BINARY_DIR}/Ethereality
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:GroupOnlyHub> ${CMAKE_BINARY_DIR}/Ethereality
	COMMAND ${CMAKE_COMMAND} -E copy
		$<TARGET_FILE:imgui-dx11> ${CMAKE_BINARY_DIR}/Ethereality
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/Ethereality/assets
	COMMAND_EXPAND_LISTS
)
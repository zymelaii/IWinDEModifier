set(UTIL_NAME DesktopTab)

add_executable(${UTIL_NAME} handler.cpp)
target_link_libraries(${UTIL_NAME} PRIVATE imgui-dx11-static)

if (NOT MSVC)
	target_link_options(${UTIL_NAME} PRIVATE --static -Wl,-subsystem,windows)
endif()
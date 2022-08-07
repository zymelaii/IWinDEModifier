file(GLOB_RECURSE HEADER_FILES *.h)
file(GLOB SOURCE_FILES iwindeapi/*.cpp)

add_library(iwindeapi SHARED ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(iwindeapi PRIVATE crypt32 bcrypt)

add_library(iwindeapi-static STATIC ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(iwindeapi-static PRIVATE crypt32 bcrypt)

file(GLOB SOURCE_FILES imgui/*.cpp)
add_library(imgui-dx11 STATIC ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(imgui-dx11 PRIVATE gdi32 dwmapi d3dcompiler d3d11)

file(GLOB SOURCE_FILES *.cpp)
add_executable(IWinDEModifier ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(IWinDEModifier PRIVATE iwindeapi-static imgui-dx11)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
	message(STATUS "BuildType: ${CMAKE_BUILD_TYPE}")
	target_compile_options(imgui-dx11 PRIVATE -DNDEBUG -O3)
	target_compile_options(IWinDEModifier PRIVATE --static -DNDEBUG -O3)
	target_link_options(IWinDEModifier PRIVATE -Wl,-subsystem,windows)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_options(imgui-dx11 PRIVATE -g -DDEBUG)
	target_compile_options(IWinDEModifier PRIVATE -g -DDEBUG)
endif()

file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
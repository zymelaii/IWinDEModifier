file(GLOB_RECURSE HEADER_FILES *.h)
file(GLOB_RECURSE SOURCE_FILES *.cpp)

set(Targets imgui-dx11-static imgui-dx11)
add_library(imgui-dx11-static STATIC ${HEADER_FILES} ${SOURCE_FILES})
add_library(imgui-dx11 SHARED ${HEADER_FILES} ${SOURCE_FILES})

foreach(target ${Targets})
	target_link_libraries(${target} PRIVATE gdi32 dwmapi d3dcompiler d3d11)
endforeach()
add_subdirectory(tinyico)

file(GLOB HEADER_FILES *.h)
file(GLOB SOURCE_FILES *.cpp proxy/*.cpp)

set(target iwde-utils)
add_library(${target} STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(${target} PRIVATE imgui-dx11 tinyico)
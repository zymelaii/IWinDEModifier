file(GLOB HEADER_FILES *.h)
file(GLOB SOURCE_FILES *.cpp)

set(Targets iwindeapi-static iwindeapi)
add_library(iwindeapi-static STATIC ${HEADER_FILES} ${SOURCE_FILES})
add_library(iwindeapi SHARED ${HEADER_FILES} ${SOURCE_FILES})

foreach(target ${Targets})
	target_link_libraries(${target} PRIVATE crypt32 bcrypt)
endforeach()
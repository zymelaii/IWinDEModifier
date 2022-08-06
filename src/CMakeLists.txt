file(GLOB_RECURSE SOURCE_FILES . *.cpp)
file(GLOB_RECURSE HEADER_FILES . *.h)

# add_library(iwindeapi SHARED ${HEADER_FILES} ${SOURCE_FILES})
add_library(iwindeapi STATIC ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(iwindeapi PRIVATE crypt32 bcrypt)
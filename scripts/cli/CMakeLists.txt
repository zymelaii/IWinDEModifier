file(GLOB_RECURSE SOURCE_FILES . *.cpp)
file(GLOB_RECURSE HEADER_FILES . *.h)
list(FILTER SOURCE_FILES EXCLUDE REGEX "cli.cpp")

add_executable(cli cli.cpp ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(cli PRIVATE iwindeapi)
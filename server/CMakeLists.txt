project(sampvoice-server)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE sampvoice_source_list "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(${PROJECT_NAME} SHARED ${sampvoice_source_list})

include_directories("include")

if(WIN32)
	add_definitions(-DNOMINMAX)
else()
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-format-security -Wno-conversion-null -Wno-pointer-arith -static-libstdc++")
	set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS " -fno-strict-aliasing")
	set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " -lrt")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "sampvoice")

target_link_libraries(${PROJECT_NAME} PRIVATE
	OMP-SDK
	NetCode
)

if(UNIX AND NOT APPLE)
	add_compile_definitions(LINUX)
endif()

GroupSourcesByFolder(${PROJECT_NAME})

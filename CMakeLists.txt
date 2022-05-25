cmake_minimum_required(VERSION 3.19)
project(sampvoice)

set(CMAKE_CXX_STANDARD 17)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

function(GroupSourcesByFolder target)
	set(SOURCE_GROUP_DELIMITER "/")
	set(last_dir "")
	set(files "")

	get_target_property(sources ${target} SOURCES)

	foreach(file ${sources})
		file(RELATIVE_PATH relative_file "${CMAKE_CURRENT_SOURCE_DIR}" ${file})
		get_filename_component(dir "${relative_file}" PATH)
		if(NOT "${dir}" STREQUAL "${last_dir}")
			if(files)
				source_group("${last_dir}" FILES ${files})
			endif()
			set(files "")
		endif()
		set(files ${files} ${file})
		set(last_dir "${dir}")
	endforeach()

	if(files)
		source_group("${last_dir}" FILES ${files})
	endif()
endfunction()

add_subdirectory(lib/omp-sdk)
add_subdirectory(lib/omp-netcode)
add_subdirectory(server)

set_property(TARGET OMP-SDK PROPERTY FOLDER "Server")
set_property(TARGET NetCode PROPERTY FOLDER "Server")
set_property(TARGET sampvoice-server PROPERTY FOLDER "Server")

if(WIN32)
    add_subdirectory(client)
endif()

set_property(TARGET sampvoice-client PROPERTY FOLDER "Client")

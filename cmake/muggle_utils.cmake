#########################################
# muggle_add_project
# descript: add a new project that all files in a folder and subfolder of 
# the folder, and automaticlly add folder structure in visual studio
# @name: the name of project
# @folder: the root path of source files
# @type: EXE, STATIC or SHARED
function(muggle_add_project name folder type)

	# files
	file(GLOB_RECURSE  ${name}_h ${folder}/*.h)
	file(GLOB_RECURSE  ${name}_c ${folder}/*.c)
	file(GLOB_RECURSE  ${name}_cpp ${folder}/*.cpp)
    file(GLOB_RECURSE  ${name}_hpp ${folder}/*.hpp)
    file(GLOB_RECURSE  ${name}_cxx ${folder}/*.cxx)
	file(GLOB_RECURSE  ${name}_cc ${folder}/*.cc)
    file(GLOB ${name}_src ${${name}_c} ${${name}_cpp} ${${name}_hpp} ${${name}_cxx} ${${name}_cc})

    # add project
    if (${type} STREQUAL EXE)
        if (WIN32)
	    	add_executable(${name}
	    		${${name}_h}
	    		${${name}_src}
	    	)
	    else()
	    	add_executable(${name}
	    		${${name}_src}
	    	)
	    endif()
    elseif (${type} STREQUAL STATIC OR ${type} STREQUAL SHARED)
        if (WIN32)
	    	add_library(${name} ${type}
	    		${${name}_h}
	    		${${name}_src}
	    	)
			# windows dll export and import
			if (${type} STREQUAL SHARED)
				target_compile_definitions(${name}
					PUBLIC ${name}_USE_DLL
					PRIVATE ${name}_EXPORTS 
				)
			endif()
	    else()
	    	add_library(${name} ${type}
	    		${${name}_src}
	    	)
	    endif()
    else()
        message(FATAL_ERROR "muggle_add_project only accept 3 type args: EXE, STATIC and SHARED")
    endif()

    # add folder structure in vs
    set(${name}_all_files ${${name}_src} ${${name}_h})
    foreach(file ${${name}_all_files})
        file(RELATIVE_PATH rel_path ${folder} ${file})
        get_filename_component(file_dir ${rel_path} DIRECTORY)
        if (NOT ${file_dir} EQUAL "")
            list(FIND ${name}_all_dirs ${file_dir} idx)
            if (${idx} EQUAL -1)
				list(APPEND ${name}_all_dirs ${file_dir})
			endif()
			list(APPEND ${file_dir}_src ${file})

			if (${file} MATCHES ".h$")
				list(APPEND ${file_dir}_h ${file})
			endif()
        else()
            if (${file} MATCHES ".h$")
				list(APPEND empty_dir_h ${file})
			endif()
        endif()
    endforeach()
    foreach(dir ${${name}_all_dirs})
		string(REPLACE "/" "\\" GROUP "${dir}")
		source_group(${GROUP} FILES ${${dir}_src})
	endforeach()

endfunction(muggle_add_project)
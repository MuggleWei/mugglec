#########################################
# muggle_add_project_with_files
# descript: add a new project that files that have the same root folder
# and automaticlly add folder structure in visual studio
# @name: the name of project
# @folder: the root path of source files
# @type: EXE, STATIC or SHARED
# @src_files: c, cpp, cxx, cc, hpp files
# @h_files: h files
function(muggle_add_project_with_files name folder type src_files h_files)

	# add project
    if (${type} STREQUAL EXE)
        if (WIN32)
	    	add_executable(${name}
	    		${src_files}
	    		${h_files}
	    	)
	    else()
	    	add_executable(${name}
	    		${src_files}
	    	)
	    endif()
    elseif (${type} STREQUAL STATIC OR ${type} STREQUAL SHARED)
        if (WIN32)
	    	add_library(${name} ${type}
	    		${src_files}
	    		${h_files}
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
	    		${src_files}
	    	)
	    endif()
    else()
        message(FATAL_ERROR "muggle_add_project_with_files only accept 3 type args: EXE, STATIC and SHARED")
    endif()

	# add folder structure in vs
    set(${name}_all_files ${src_files} ${h_files})
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

endfunction(muggle_add_project_with_files)

#########################################
# muggle_add_project
# descript: add a new project that all files in a folder and subfolder of 
# the folder, and automaticlly add folder structure in visual studio
# @name: the name of project
# @folder: the root path of source files
# @type: EXE, STATIC or SHARED
function(muggle_add_project name folder type)

	# files
	file(GLOB_RECURSE h_files ${folder}/*.h)
	file(GLOB_RECURSE c_files ${folder}/*.c)
	file(GLOB_RECURSE cpp_files ${folder}/*.cpp)
    file(GLOB_RECURSE hpp_files ${folder}/*.hpp)
    file(GLOB_RECURSE cxx_files ${folder}/*.cxx)
	file(GLOB_RECURSE cc_files ${folder}/*.cc)
    list(APPEND src_files ${c_files} ${cpp_files} ${hpp_files} ${cxx_files} ${cc_files})

	muggle_add_project_with_files(${name} ${folder} ${type} "${src_files}" "${h_files}")

endfunction(muggle_add_project)

#########################################
# muggle_add_project
# descript: add a new project that all files in folders that are the 
# subfolder of root_folder, and automaticlly add folder structure in
# visual studio
# @name: the name of project
# @root_folder: the root path of source files
# @type: EXE, STATIC or SHARED
# @folders: the subfolder of root_folder
function(muggle_add_project_with_folders name root_folder type folders)

	set(src_files "")
	set(h_files "")

	# ensure all folder in folders is subpath of root_folder
	foreach(folder ${folders})
		file(RELATIVE_PATH rel_path ${root_folder} ${folder})
		if (${rel_path} MATCHES "^\\.\\.")
			message(FATAL_ERROR "${folder} is not the sub path of the ${root_folder}")
		endif()

		file(GLOB  tmp_h ${folder}/*.h)
		file(GLOB  tmp_c ${folder}/*.c)
		file(GLOB  tmp_cpp ${folder}/*.cpp)
    	file(GLOB  tmp_hpp ${folder}/*.hpp)
    	file(GLOB  tmp_cxx ${folder}/*.cxx)
		file(GLOB  tmp_cc ${folder}/*.cc)
    	list(APPEND tmp_src ${tmp_c} ${tmp_cpp} ${tmp_hpp} ${tmp_cxx} ${tmp_cc})

		list(APPEND src_files ${tmp_src})
		list(APPEND h_files ${tmp_h})

	endforeach(folder)

	# add project
	muggle_add_project_with_files(${muggle_add_project_with_folders} ${name} ${root_folder} ${type} "${src_files}" "${h_files}")
	
endfunction(muggle_add_project_with_folders)
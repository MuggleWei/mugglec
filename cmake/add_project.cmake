include(${CMAKE_CURRENT_LIST_DIR}/muggle_utils.cmake)

# add muggle library in folder recusively
function(addMuggleLib name is_pure_c export_macros folder_name link_targets dependencies)

	# add project
	muggle_add_project(${name} ${muggle_source_dir}/muggle/${folder_name} ${MUGGLE_LIB_TYPE})

	# property
	if (${is_pure_c})
		set_target_properties(${name} PROPERTIES 
			LINKER_LANGUAGE C
		)
	endif()
	
	# link
	if (NOT link_targets STREQUAL "")
		target_link_libraries(${name} 
			${link_targets}
		)
	endif()

	# dependencies
	if (NOT dependencies STREQUAL "")
		add_dependencies(${name}
			${dependencies}
		)
	endif()

	# windows dll export and import
	if(MSVC AND ${MUGGLE_BUILD_SHARED_LIB})
		target_compile_definitions(${name}
			PUBLIC MUGGLE_USE_DLLS
			PRIVATE ${export_macros} 
		)
	endif()

	# install headers
	file(GLOB_RECURSE h_files ${muggle_source_dir}/muggle/${folder_name}/*.h)
	file(GLOB_RECURSE hpp_files ${muggle_source_dir}/muggle/${folder_name}/*.hpp)
	set(headers ${h_files} ${hpp_files})
	foreach(file ${headers})
		file(RELATIVE_PATH rel_path ${muggle_source_dir}/muggle/${folder_name} ${file})
		get_filename_component(file_dir ${rel_path} DIRECTORY)
		if (NOT ${file_dir} EQUAL "")
			list(FIND all_dirs ${file_dir} idx)
			if (${idx} EQUAL -1)
				list(APPEND all_dirs ${file_dir})
			endif()
			list(APPEND ${file_dir}_h ${file})
		else()
			list(APPEND empty_dir_h ${file})
		endif()
	endforeach()

	foreach(file_dir ${all_dirs})
		install(FILES ${${file_dir}_h} DESTINATION ${MUGGLE_INSTALL_INCLUDE_PATH}/${folder_name}/${file_dir})
	endforeach()
	install(FILES ${empty_dir_h} DESTINATION ${MUGGLE_INSTALL_INCLUDE_PATH}/${folder_name})

	# instal binary files
	install(TARGETS ${name}
		RUNTIME DESTINATION ${MUGGLE_INSTALL_RUNTIME_PATH}
		LIBRARY DESTINATION ${MUGGLE_INSTALL_LIBRARY_PATH}
		ARCHIVE DESTINATION ${MUGGLE_INSTALL_ARCHIVE_PATH}
	)	

endfunction(addMuggleLib)

# add simple project
function(addProject name folder_name dependencies type)

	# add project
	muggle_add_project(${name} ${folder_name} ${type})
	
	if (NOT dependencies STREQUAL "")
		target_link_libraries(${name} 
			${dependencies}
		)
		add_dependencies(${name}
			${dependencies}
		)
	endif()

endfunction(addProject) 
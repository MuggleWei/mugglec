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
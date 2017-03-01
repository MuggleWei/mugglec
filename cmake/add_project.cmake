# add muggle library in single folder
function(addMuggleLib name is_pure_c export_macro folder_name link_targets dependencies)

	# files
	file(GLOB ${name}_h ${muggle_source_dir}/muggle/${folder_name}/*.h)
	file(GLOB ${name}_c ${muggle_source_dir}/muggle/${folder_name}/*.c)
	file(GLOB ${name}_cpp ${muggle_source_dir}/muggle/${folder_name}/*.cpp)
	
	if (${is_pure_c})
		file(GLOB ${name}_src 
			${${name}_c}
		)
	else()
		file(GLOB ${name}_src 
			${${name}_c}
			${${name}_cpp}
		)
	endif()
	
	# add lib
	if (WIN32)
		add_library(${name} ${MUGGLE_LIB_TYPE}
			${${name}_h}
			${${name}_src}
		)
	else()
		add_library(${name} ${MUGGLE_LIB_TYPE}
			${${name}_src}
		)
	endif()

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
			PRIVATE ${export_macro} 
		)
	endif()

	# install
	install(FILES ${${name}_h} DESTINATION ${MUGGLE_INSTALL_INCLUDE_PATH}/${folder_name})
	install(TARGETS ${name}
		RUNTIME DESTINATION ${MUGGLE_INSTALL_RUNTIME_PATH}
		LIBRARY DESTINATION ${MUGGLE_INSTALL_LIBRARY_PATH}
		ARCHIVE DESTINATION ${MUGGLE_INSTALL_ARCHIVE_PATH}
	)

endfunction(addMuggleLib)

# add muggle library in folder recusively
function(addMuggleLibRecurs name is_pure_c export_macros folder_name link_targets dependencies)

	# files
	if(NOT ${folder_name} EQUAL "")
		file(GLOB_RECURSE  ${name}_h ${muggle_source_dir}/muggle/${folder_name}/*.h)
		file(GLOB_RECURSE  ${name}_c ${muggle_source_dir}/muggle/${folder_name}/*.c)
		file(GLOB_RECURSE  ${name}_cpp ${muggle_source_dir}/muggle/${folder_name}/*.cpp)
	else()
		file(GLOB_RECURSE  ${name}_h ${muggle_source_dir}/muggle/*.h)
		file(GLOB_RECURSE  ${name}_c ${muggle_source_dir}/muggle/*.c)
		file(GLOB_RECURSE  ${name}_cpp ${muggle_source_dir}/muggle/*.cpp)
	endif()
	
	if (${is_pure_c})
		file(GLOB ${name}_src 
			${${name}_c}
		)
	else()
		file(GLOB ${name}_src 
			${${name}_c}
			${${name}_cpp}
		)
	endif()
	
	# add lib
	if (WIN32)
		add_library(${name} ${MUGGLE_LIB_TYPE}
			${${name}_h}
			${${name}_src}
		)
	else()
		add_library(${name} ${MUGGLE_LIB_TYPE}
			${${name}_src}
		)
	endif()

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

	# put all directories and files into list
	set(${name}_all_files ${${name}_src} ${${name}_h})
	foreach(file ${${name}_all_files})
		file(RELATIVE_PATH rel_path ${muggle_source_dir}/muggle/${folder_name} ${file})
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

	# source group and install head
	foreach(dir ${${name}_all_dirs})
		string(REPLACE "/" "\\" GROUP "${dir}")
		source_group(${GROUP} FILES ${${dir}_src})
		install(FILES ${${name}_h} DESTINATION ${MUGGLE_INSTALL_INCLUDE_PATH}/${folder_name}/${dir})
	endforeach()
	install(FILES ${empty_dir_h} DESTINATION ${MUGGLE_INSTALL_INCLUDE_PATH}/${folder_name})

	# install binary files
	install(TARGETS ${name}
		RUNTIME DESTINATION ${MUGGLE_INSTALL_RUNTIME_PATH}
		LIBRARY DESTINATION ${MUGGLE_INSTALL_LIBRARY_PATH}
		ARCHIVE DESTINATION ${MUGGLE_INSTALL_ARCHIVE_PATH}
	)	

endfunction(addMuggleLibRecurs)

# add simple project
function(addProject name folder_name dependencies type)
	file(GLOB ${name}_H ${folder_name}/*.h)
	file(GLOB ${name}_C ${folder_name}/*.c)
	file(GLOB ${name}_CPP ${folder_name}/*.cpp)
	
	if (type STREQUAL "EXE")
		if (WIN32)
			add_executable(${name}
				${${name}_H}
				${${name}_C}
				${${name}_CPP}
			)
		else()
			add_executable(${name}
				${${name}_C}
				${${name}_CPP}
			)
		endif()
	elseif(type STREQUAL "DLL")
		if (WIN32)
			add_library(${name} SHARED
				${${name}_H}
				${${name}_C}
				${${name}_CPP}
			)
			# windows dll export and import
			target_compile_definitions(${name}
				PUBLIC ${name}_USE_DLL
				PRIVATE ${name}_EXPORTS 
			)
		else()
			add_library(${name} SHARED
				${${name}_C}
				${${name}_CPP}
			)
		endif()
	else()
		if (WIN32)
			add_library(${name}
				${${name}_H}
				${${name}_C}
				${${name}_CPP}
			)
		else()
			add_library(${name}
				${${name}_C}
				${${name}_CPP}
			)
		endif()
	endif()
	
	if (NOT dependencies STREQUAL "")
		target_link_libraries(${name} 
			${dependencies}
		)
		add_dependencies(${name}
			${dependencies}
		)
	endif()
endfunction(addProject) 
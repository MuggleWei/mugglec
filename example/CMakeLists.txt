cmake_minimum_required(VERSION 2.8.9)

addProject(ExampleCLog   	${CMAKE_CURRENT_LIST_DIR}/clog "${muggle_base_c}" EXE)
addProject(ProConMtxCond 	${CMAKE_CURRENT_LIST_DIR}/pro_con_mtx_cond "${muggle_base_c}" EXE)
addProject(ExampleSem    	${CMAKE_CURRENT_LIST_DIR}/sem "${muggle_base_c}" EXE)
addProject(ExampleSingleton ${CMAKE_CURRENT_LIST_DIR}/singleton "${muggle_utils_cpp}" EXE)

# folder
set_target_properties(
	ExampleCLog ProConMtxCond ExampleSem ExampleSingleton
	PROPERTIES
	FOLDER "Example"
)
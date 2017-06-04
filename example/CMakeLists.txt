cmake_minimum_required(VERSION 2.8.9)

# log
addProject(ExampleCLog   ${CMAKE_CURRENT_LIST_DIR}/clog "${muggle_base_c}" EXE)
addProject(ProConMtxCond ${CMAKE_CURRENT_LIST_DIR}/pro_con_mtx_cond "${muggle_base_c}" EXE)
addProject(ExampleSem    ${CMAKE_CURRENT_LIST_DIR}/sem "${muggle_base_c}" EXE)

# folder
set_target_properties(
	ExampleCLog ProConMtxCond ExampleSem
	PROPERTIES
	FOLDER "Example"
)
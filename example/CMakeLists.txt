cmake_minimum_required(VERSION 3.0.2)

muggle_add_project(ExampleCLog "${CMAKE_CURRENT_LIST_DIR}/clog" EXE)
target_link_libraries(ExampleCLog ${muggle_cc})

muggle_add_project(ProConMtxCond "${CMAKE_CURRENT_LIST_DIR}/pro_con_mtx_cond" EXE)
target_link_libraries(ProConMtxCond ${muggle_cc})

muggle_add_project(ExampleSem "${CMAKE_CURRENT_LIST_DIR}/sem" EXE)
target_link_libraries(ExampleSem ${muggle_cc})

muggle_add_project(ExampleSingleton "${CMAKE_CURRENT_LIST_DIR}/singleton" EXE)
target_link_libraries(ExampleSingleton ${muggle_cc})

muggle_add_project(ExampleTunnel "${CMAKE_CURRENT_LIST_DIR}/tunnel" EXE)
target_link_libraries(ExampleTunnel ${muggle_cc})

# folder
set_target_properties(
	ExampleCLog ProConMtxCond ExampleSem ExampleSingleton ExampleTunnel
	PROPERTIES
	FOLDER "example"
)
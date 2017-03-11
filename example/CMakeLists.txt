cmake_minimum_required(VERSION 2.8.9)

# log
set(example_dependencies ${muggle_base_c})
addProject("ExampleLog" "log" "${example_dependencies}" "EXE")
addProject("ProConMtxCond" "pro_con_mtx_cond" "${example_dependencies}" "EXE")
addProject("ExampleSem" "sem" "${example_dependencies}" "EXE")

# folder
set_target_properties(
	ExampleLog ProConMtxCond ExampleSem
	PROPERTIES
	FOLDER "Example"
)
cmake_minimum_required(VERSION 2.8.9)

# log
set(example_log_dependencies ${muggle_base_c})
addProject("ExampleLog" "log" "${example_log_dependencies}" "EXE")
addProject("ProConMtxCond" "pro_con_mtx_cond" "${example_log_dependencies}" "EXE")

# folder
set_target_properties(
	ExampleLog ProConMtxCond
	PROPERTIES
	FOLDER "Example"
)
cmake_minimum_required(VERSION 2.8.9)

# empty dll
set(run_test_dependencies "")
addProject("RunTestEmpty" "run_test_empty" "${run_test_dependencies}" "DLL")

# test base
set(run_test_dependencies ${muggle_base})
addProject("RunTestBase" "run_test_base" "${run_test_dependencies}" "EXE")

# test memory pool
set(run_test_dependencies ${muggle_memory_pool} ${muggle_cpp_utils})
addProject("RunTestMemPool" "run_test_mem_pool" "${run_test_dependencies}" "EXE")

# test cpp utils
set(run_test_dependencies ${muggle_cpp_utils})
addProject("RunTestCppUtils" "run_test_cpp_utils" "${run_test_dependencies}" "EXE")

# folder
set_target_properties(
	RunTestEmpty RunTestBase RunTestMemPool RunTestCppUtils
	PROPERTIES
	FOLDER "RunTest"
)
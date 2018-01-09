cmake_minimum_required(VERSION 3.0.2)

muggle_add_project(BenchmarkMemoryPool "${CMAKE_CURRENT_LIST_DIR}/memory_pool" EXE)
target_link_libraries(BenchmarkMemoryPool ${muggle_cc})

# folder
set_target_properties(
	BenchmarkMemoryPool
	PROPERTIES
	FOLDER "benchmark"
)
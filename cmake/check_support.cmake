include(CheckIncludeFile)
include(CheckCSourceCompiles)
include(CheckSymbolExists)
include(CMakePushCheckState)

# check with include
check_include_file(linux/futex.h MUGGLE_C_HAVE_LINUX_FUTEX)
check_include_file(sys/futex.h MUGGLE_C_HAVE_SYS_FUTEX)

# check with compile
check_c_source_compiles("
	#include <stdalign.h>
	int main() {
		alignas(128) int data;
		return 0;
	}
" MUGGLE_C_HAVE_ALIGNAS)

check_c_source_compiles("
	#include <stdlib.h>
	int main() {
		void *p = aligned_alloc(128, 128 * 16);
		free(p);
		return 0;
	}
" MUGGLE_C_HAVE_ALIGNED_ALLOC)

check_c_source_compiles("
	#include <sys/mman.h>
	int main() {
		int flags = 0;
		flags |= MAP_HUGE_2MB;
		flags |= MAP_HUGE_1GB;
		return 0;
	}
" MUGGLE_C_HAVE_MAP_HUGE_NBYTES)

check_c_source_compiles("
	#include <sys/ipc.h>
	#include <sys/shm.h>
	int main() {
		int flags = 0;
		flags |= SHM_HUGE_2MB;
		flags |= SHM_HUGE_1GB;
		return 0;
	}
" MUGGLE_C_HAVE_SHM_HUGE_NBYTES)

if (MSVC)

check_c_source_compiles("
	#include <memoryapi.h>
	int main() {
		int flags = 0;
		flags |= FILE_MAP_LARGE_PAGES;
		return 0;
	}
" MUGGLE_C_HAVE_MSVC_LARGE_PAGE)

endif()

# check symbol
check_symbol_exists(MADV_HUGEPAGE "sys/mman.h"
	MUGGLE_C_HAVE_MADV_HUGEPAGE)
check_symbol_exists(MADV_POPULATE_WRITE "sys/mman.h"
	MUGGLE_C_HAVE_MADV_POPULATE_WRITE)

if (MSVC)
	cmake_push_check_state()

	set(CMAKE_REQUIRED_LIBRARIES kernelbase)
	check_symbol_exists(VirtualAlloc2
		"windows.h;memoryapi.h" MUGGLE_C_HAVE_VIRTUALALLOC2)

	cmake_pop_check_state()
endif()

# detech endianness
include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)  # NOTE: big endian is 1, little endian is 0
if (IS_BIG_ENDIAN)
	set(MUGGLE_C_IS_BIG_ENDIAN 1)
else()
	set(MUGGLE_C_IS_BIG_ENDIAN 0)
endif()

if (CMAKE_HOST_UNIX)
	find_package(Backtrace)
	if (Backtrace_FOUND)
		#message("Found Backtrace")
		#message("Backtrace header: ${Backtrace_HEADER}")
		#message("Backtrace libs: ${Backtrace_LIBRARIES}")

		set(MUGGLE_C_HAVE_BACKTRACE 1)
		set(MUGGLE_C_BACKTRACE_HEADER ${Backtrace_HEADER})
	endif()
endif()

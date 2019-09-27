#include "muggle/c/muggle_c.h"

int main()
{
	char process_path[MUGGLE_MAX_PATH];
	char process_dir[MUGGLE_MAX_PATH];
	char working_dir[MUGGLE_MAX_PATH];
#if MUGGLE_PLATFORM_WINDOWS
	const char *root_dir = "c:/";
#else
	const char *root_dir = "/";
#endif

	// get process path
	muggle_os_process_path(process_path, sizeof(process_path));
	muggle_path_dirname(process_path, process_dir, sizeof(process_dir));

	// get working dir
	muggle_os_curdir(working_dir, sizeof(working_dir));

	printf("launch process: \nprocess path: %s\nprocess dir: %s\nworking dir: %s\n",
		process_path, process_dir, working_dir);

	// chdir
	muggle_os_chdir(root_dir);
	muggle_os_curdir(working_dir, sizeof(working_dir));
	printf("chdir: %s\n", working_dir);

	return 0;
}

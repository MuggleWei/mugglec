#include "muggle/c/muggle_c.h"

void output_files(muggle_file_list_node_t *node)
{
	while (node) {
		fprintf(stdout, "%s\n", node->filepath);
		node = (muggle_file_list_node_t *)node->next;
	}
	fprintf(stdout, "\n");
}

int main(int argc, char *argv[])
{
	const char *dirpath = ".";
	if (argc > 1) {
		dirpath = argv[1];
	}

	muggle_file_list_node_t *nodes = NULL;

	fprintf(stdout, "--------------------------------\n");
	fprintf(stdout, "list all files\n");
	fprintf(stdout, "--------------------------------\n");
	nodes = muggle_os_listdir(dirpath, MUGGLE_FILE_TYPE_NULL);
	output_files(nodes);
	muggle_os_free_file_nodes(nodes);

	fprintf(stdout, "--------------------------------\n");
	fprintf(stdout, "list all files (regular file)\n");
	fprintf(stdout, "--------------------------------\n");
	nodes = muggle_os_listdir(dirpath, MUGGLE_FILE_TYPE_REGULAR);
	output_files(nodes);
	muggle_os_free_file_nodes(nodes);

	fprintf(stdout, "--------------------------------\n");
	fprintf(stdout, "list all files (directory)\n");
	fprintf(stdout, "--------------------------------\n");
	nodes = muggle_os_listdir(dirpath, MUGGLE_FILE_TYPE_DIR);
	output_files(nodes);
	muggle_os_free_file_nodes(nodes);

	return 0;
}

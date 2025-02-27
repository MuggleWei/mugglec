#include "muggle/c/muggle_c.h"

int main()
{
    const char *filepath = "tmp/hello.txt";
    const char *new_filepath = "tmp/hello_world.txt";

    char cur_dir[MUGGLE_MAX_PATH];
    if (muggle_os_curdir(cur_dir, sizeof(cur_dir)) != 0) {
        fprintf(stderr, "failed get current dir\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "current directory: %s\n", cur_dir);

    if (muggle_path_exists(new_filepath)) {
        if (muggle_os_remove(new_filepath) != 0) {
            fprintf(stderr, "failed remove filepath: %s\n", new_filepath);
            exit(EXIT_FAILURE);
        } else {
            fprintf(stdout, "success remove filepath: %s\n", new_filepath);
        }
    }

    if (!muggle_path_exists(filepath)) {
        FILE *fp = muggle_os_fopen(filepath, "wb");
        if (fp == NULL) {
            fprintf(stderr, "failed open filepath: %s\n", filepath);
            exit(EXIT_FAILURE);
        } else {
            fprintf(stdout, "success open filepath: %s\n", filepath);
            fclose(fp);
        }
    } else {
        fprintf(stdout, "filepath exists: %s\n", filepath);
    }

    int ret = muggle_os_rename(filepath, new_filepath);
    if (ret != 0) {
        fprintf(stderr, "failed rename\n");
    } else {
        fprintf(stdout, "success rename %s -> %s\n", filepath, new_filepath);
    }

    return 0;
}
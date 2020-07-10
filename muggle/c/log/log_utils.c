#include "log_utils.h"
#include "muggle/c/log/log_handle_console.h"
#include "muggle/c/log/log_handle_rotating_file.h"
#include "muggle/c/os/path.h"
#include "muggle/c/os/os.h"

static muggle_log_handle_t s_log_utils_handle_console;
static muggle_log_handle_t s_log_utils_handle_rotating_file;

int muggle_log_simple_init(int level_console, int level_file_rotating)
{
    // init console log
    muggle_log_handle_console_init(
        &s_log_utils_handle_console,
        MUGGLE_LOG_WRITE_TYPE_SYNC,
        MUGGLE_LOG_FMT_LEVEL | MUGGLE_LOG_FMT_FILE | MUGGLE_LOG_FMT_FUNC,
        level_console,
        0, NULL, NULL, 1);
    muggle_log_add_handle(&s_log_utils_handle_console);

    // init file rotating log
    char process_path[MUGGLE_MAX_PATH];
    if (muggle_os_process_path(process_path, sizeof(process_path)) != 0)
    {
        MUGGLE_LOG_ERROR("failed get process path");
        return -1;
    }

    char process_dir[MUGGLE_MAX_PATH];
    if (muggle_path_dirname(process_path, process_dir, sizeof(process_dir)) != 0)
    {
        MUGGLE_LOG_ERROR("failed get dirname from path: %s", process_path);
        return -1;
    }

    char process_name[MUGGLE_MAX_PATH];
    if (muggle_path_basename(process_path, process_name, sizeof(process_name)) != 0)
    {
        MUGGLE_LOG_ERROR("failed get basename from path: %s", process_path);
        return -1;
    }

    char log_dir[MUGGLE_MAX_PATH];
    if (muggle_path_join(process_dir, "log", log_dir, sizeof(log_dir)) != 0)
    {
        MUGGLE_LOG_ERROR("failed join log dir");
        return -1;
    }

    if (!muggle_path_exists(log_dir))
    {
        MUGGLE_LOG_INFO("log dir not exists, mkdir %s", log_dir);
        if (muggle_os_mkdir(log_dir) != 0)
        {
            MUGGLE_LOG_ERROR("failed mkdir: %s", log_dir);
            return -1;
        }
    }

    char log_name[128];
    snprintf(log_name, sizeof(log_name), "%s.log", process_name);

    char log_path[MUGGLE_MAX_PATH];
    if (muggle_path_join(log_dir, log_name, log_path, sizeof(log_path)) != 0)
    {
        MUGGLE_LOG_ERROR("failed join log path");
        return -1;
    }

    int ret = muggle_log_handle_rotating_file_init(
        &s_log_utils_handle_rotating_file,
        MUGGLE_LOG_WRITE_TYPE_SYNC,
        MUGGLE_LOG_FMT_LEVEL | MUGGLE_LOG_FMT_FILE | MUGGLE_LOG_FMT_TIME | MUGGLE_LOG_FMT_TIME | MUGGLE_LOG_FMT_THREAD,
        level_file_rotating,
        0, NULL, NULL, log_path,
        1024 * 1024 * 64, 5
    );
    if (ret != 0)
    {
        MUGGLE_LOG_ERROR("failed init log rotating file handle with path: %s", log_path);
        return -1;
    }

    muggle_log_add_handle(&s_log_utils_handle_rotating_file);

    return 0;
}

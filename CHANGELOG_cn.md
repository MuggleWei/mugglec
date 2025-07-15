# 变更记录 
[English](./CHANGELOG.md) | 中文

| 日期 | 版本 |
| ---- | ---- |
| 2025-07-15 | [v2.1.2](#v212) |
| 2025-05-13 | [v2.1.1](#v211) |
| 2025-04-28 | [v2.1.0](#v210) |
| 2025-02-27 | [v2.0.2](#v202) |
| 2025-02-24 | [v2.0.1](#v201) |
| 2025-02-23 | [v2.0.0](#v200) |

## v2.1.2
* 修复: 默认使用 `poll` 来等待 TCP 连接完成, 修复当 `fd` 值大等于 1024 时, 错误将正常连接判断为超时的问题
* 修复: 在 `muggle_socket_evloop_on_accept` 当中处理错误码 `EMFILE`

## v2.1.1
* 修复 build.bat 脚本中传入错误的 `BUILD_TYPE` 的问题
* 更新 `muggle_tcp_bind`, 设置 `SO_REUSEADDR`

## v2.1.0
* 新增 `muggle_socket_blocking_write` 以支持带超时时间的阻塞写

## v2.0.2
* 修复在 windows 下, 操作非 ASCII 字符路径出错的问题

## v2.0.1
* 增加了 `MUGGLE_EV_LOOP_EXIT_STATUS`, 非本线程调用 `evloop_exit` 时不直接将状态设为退出, 防止出现 **heap-use-after-free**

## v2.0.0
相对于 **release_1.x** 的最后一个版本 **v1.10.3**, 做出了如下变更
* 调整了部分结构体的内存布局和字段, 以修复已知的问题和实现更好的效率
* 修改了部分以前漏掉 `MUGGLE_`/`muggle_` 前缀的宏和函数的命名, 以防止和其他库中的符号出现冲突
* 修复 `muggle_nsleep` 中可能出现的 bug 并更改其入参类型
* 修复 `muggle_event_loop_t` 中, 定时器受到其他消息触发的影响, 可能出现间隔错误的问题
* 修改日志中的线程号为 `muggle_thread_readable_id` 类型, 以方便日志的阅读
* 废弃 `muggle_bytes_buffer_writer_move`, 转而使用 `muggle_bytes_buffer_writer_move_n`, 以修复 `muggle_bytes_buffer_writer_fc` 和 `muggle_bytes_buffer_writer_move` 传入的 `num_bytes` 不一致而可能导致的 bug
* 新增 `muggle_memory_pool_set_max_delta_cap`, 以修复 `muggle_memory_pool_t` 在扩展内存时可能出现的 bug
* 新增 `muggle_socket_evloop_pipe_t` 以支持其他线程对 `muggle_event_loop_t` 的通知功能
* 新增 `muggle_socket_block_write` 以支持阻塞写
* 新增 `muggle_os_listdir` 和 `muggle_os_free_file_nodes` 以支持对文件夹的遍历
* 新增 `muggle_shm_t` 和 `muggle_shm_ringbuf_t` 以支持跨进程通讯
* 新增 `muggle_time_counter_t` 用于耗时的测量
* 新增 `muggle_flow_controller_t` 和 `muggle_fast_flow_controller_t` 用于网络流量控制

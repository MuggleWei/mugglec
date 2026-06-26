# 变更记录 
[English](./CHANGELOG.md) | 中文

| 日期 | 版本 |
| ---- | ---- |
| 2026-06-26 | [v2.4.1](#v241) |
| 2026-06-17 | [v2.4.0](#v240) |
| 2026-06-10 | [v2.3.0](#v230) |
| 2026-05-22 | [v2.2.1](#v221) |
| 2025-12-29 | [v2.2.0](#v220) |
| 2025-07-29 | [v2.1.4](#v214) |
| 2025-07-22 | [v2.1.3](#v213) |
| 2025-07-15 | [v2.1.2](#v212) |
| 2025-05-13 | [v2.1.1](#v211) |
| 2025-04-28 | [v2.1.0](#v210) |
| 2025-02-27 | [v2.0.2](#v202) |
| 2025-02-24 | [v2.0.1](#v201) |
| 2025-02-23 | [v2.0.0](#v200) |

## v2.4.1
* 修复 `muggle_log_file_rotate_handler_t::use_local_time` 在初始化阶段未及时设置的问题

## v2.4.0
* 新增 `muggle_time_wheel_t`

## v2.3.0
* 新增 `muggle_tcp_listen_with_cb`, 方便在回调中设置诸如 SO_SNDBUF 或 SO_RCVBUF 的值
* 新增 `muggle_tcp_connect_with_cb`, 方便在回调中设置诸如 SO_SNDBUF 或 SO_RCVBUF 的值
* 新增 `muggle_tcp_bind_connect_with_cb`, 方便在回调中设置诸如 SO_SNDBUF 或 SO_RCVBUF 的值

## v2.2.1
* 修复 `heap` 在 `cmp` 没做空指针判断时, 删除最后一个元素会导致崩溃的问题
* 更新 `muggle_array_list_destroy`, 销毁时将指针重置为 NULL
* 更新 `muggle_stack_destroy`, 销毁时将指针重置为 NULL
* 更新 `muggle_hex_from_bytes`, 使得更加缓存友好, 提升效率
* 修复 `muggle_evloop_new` 当中, 当初始化失败, 可能导致的内存泄露的问题
* 更新 `muggle_ev_signal_destroy`, 无效的 `socket_fd` 统一使用 `MUGGLE_INVALID_EVENT_FD`, 使得语义更加统一
* 修复 `muggle_memory_pool_init` 当中, 当是 32 位系统遇到非法参数, 可能导致的内存泄露问题
* 修复 `muggle_memory_pool_alloc` 当中, 出现了 `uint32_t` 溢出时, 没有对新内存大小进行检测的问题
* 修复在 Windows 平台下, `muggle_mcast_join` 当地址为 IPv6 类型, 错误的将比较写为赋值的问题
* 修复在 Windows 平台下, `muggle_socket_evloop_pipe_init` 失败时, 掉了了不正确的关闭 socket 的函数

## v2.2.0
* 更新内存池, 支持初始化时指定使用 THP(透明大页)

## v2.1.4
* 修复: 让 `muggle_tcp_bind_connect` 与 `muggle_tcp_connect` 保持一致, 使用 `muggle_wait_tcp_connect` 来等待连接准备就绪

## v2.1.3
* 更新内存池, 支持在 64 位平台一次性预分配超过 4GB 的内存
* 更新线程安全内存池, 在初始化中分配中使用 `size_t` 类型

## v2.1.2
* 修复: 默认使用 `poll` 来等待 TCP 主动连接完成, 以避免原本使用 `select` 等待会导致的当 `fd` 值大等于 1024 时, 错误的将正常连接判断为超时的问题
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

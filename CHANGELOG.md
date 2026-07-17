# ChangeLog 
English | [中文](./CHANGELOG_cn.md)

| date | version |
| ---- | ---- |
| 2026-07-17 | [v2.5.1](#v251) |
| 2026-07-06 | [v2.5.0](#v250) |
| 2026-06-27 | [v2.4.2](#v242) |
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

## v2.5.1
* Fix issue when `muggle_synclock_lock` handle CAS spurious failure

## v2.5.0
* Add `muggle_socket_evloop_pipe_read_n` to support `evloop_pipe` batch read

## v2.4.2
* Fixed issue that occurred when `muggle_memory_pool_t`/`muggle_ts_memory_pool_t` init exceeding 4GB

## v2.4.1
* Fix `muggle_log_file_rotate_handler_t::use_local_time` was not set in time during init phase

## v2.4.0
* Add `muggle_time_wheel_t`

## v2.3.0
* Add `muggle_tcp_listen_with_cb` to facilitate setting values such as SO_SNDBUF or SO_RCVBUF in callbacks.
* Add `muggle_tcp_connect_with_cb` to facilitate setting values such as SO_SNDBUF or SO_RCVBUF in callbacks.
* Add `muggle_tcp_bind_connect_with_cb` to facilitate setting values such as SO_SNDBUF or SO_RCVBUF in callbacks.

## v2.2.1
* fixed an issue where deleting the last element in `heap` would cause a crash if `cmp` didn't perform a null pointer check
* updated `muggle_array_list_destroy` to reset the pointer to NULL upon destruction
* updated `muggle_stack_destroy` to reset the pointer to NULL upon destruction
* updated `muggle_hex_from_bytes` for better cache friendliness and improved efficiency
* fixed a potential memory leak in `muggle_evloop_new` due to initialization failure
* updated `muggle_ev_signal_destroy` to consistently use `MUGGLE_INVALID_EVENT_FD` for more consistent semantics
* fixed a potential memory leak in `muggle_memory_pool_init` on 32-bit systems when encountering illegal parameters
* fixed a potential memory leak in `muggle_memory_pool_alloc`. An issue occurred where `uint32_t` overflow did not check the new memory size
* fixed an issue on Windows where `muggle_mcast_join` incorrectly wrote an assignment instead of a comparison when the address was of IPv6 type
* fixed an issue on Windows where `muggle_socket_evloop_pipe_init` failed and incorrectly called the function to close the socket

## v2.2.0
* update memory pool, support init with THP(Transparent Huge Page)

## v2.1.4
* Fixed: keep `muggle_tcp_bind_connect` and `muggle_tcp_connect` consistent, use `muggle_wait_tcp_connect` to wait for the connection be ready

## v2.1.3
* update memory pool, support preallocate >= 4GB at a time
* update threadsafe memory pool, use `size_t` in initialize allocate

## v2.1.2
* Fixed: use `poll` wait Active TCP connection completed by default, to avoid the problem that when `fd` is greater than or equal to 1024, the normal connection is mistakenly judged as timeout when using `select` wait
* Fixed: handle `EMFILE` error in `muggle_socket_evloop_on_accept`

## v2.1.1
* Fixed incorrect `BUILD_TYPE` in build.bat
* update `muggle_tcp_bind`, set `SO_REUSEADDR` enable

## v2.1.0
* Add `muggle_socket_blocking_write` to support blocking write with timeout

## v2.0.2
* Fixed issue with operating non-ASCII character paths in Windows

## v2.0.1
* Added `MUGGLE_EV_LOOP_EXIT_STATUS`, when calling `evloop_exit` from a non-current thread, the status is not set to exit directly, to avoid **heap-use-after-free**

## v2.0.0
Compared with the last version of **release_1.x**, **v1.10.3**, the following changes have been made
* Adjusted the memory layout and fields of some structures to fix known problems and achieve better efficiency
* Modified the naming of some macros and functions that previously omitted the `MUGGLE_`/`muggle_` prefix to prevent conflicts with symbols in other libraries
* Fixed possible bugs in `muggle_nsleep` and changed its input parameter type
* Fixed the problem in `muggle_event_loop_t` that the timer is affected by other message triggers and may have interval errors
* Modified the thread id type in the log to `muggle_thread_readable_id` type to facilitate log reading
* Deprecated `muggle_bytes_buffer_writer_move` and used `muggle_bytes_buffer_writer_move_n` to fix `muggle_bytes_buffer_writer_fc` and `muggle_bytes_buffer_writer_move` passes inconsistent `num_bytes` which may cause bugs
* Add `muggle_memory_pool_set_max_delta_cap` to fix the bugs that may occur when `muggle_memory_pool_t` expands memory
* Add `muggle_socket_evloop_pipe_t` to support other threads to notify `muggle_event_loop_t`
* Add `muggle_socket_block_write` to support blocking write
* Add `muggle_os_listdir` and `muggle_os_free_file_nodes` to support folder traversal
* Add `muggle_shm_t` and `muggle_shm_ringbuf_t` to support cross-process communication
* Add `muggle_time_counter_t` for time elapsed measurement
* Add `muggle_flow_controller_t` and `muggle_fast_flow_controller_t` for network flow control

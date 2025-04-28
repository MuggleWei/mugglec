# ChangeLog 
English | [中文](./CHANGELOG_cn.md)

| date | version |
| ---- | ---- |
| 2025-04-28 | [v2.1.0](#v210) |
| 2025-02-27 | [v2.0.2](#v202) |
| 2025-02-24 | [v2.0.1](#v201) |
| 2025-02-23 | [v2.0.0](#v200) |

## (Next Version)
* Fixed incorrect `BUILD_TYPE` in build.bat

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

#include "trans_message.h"

void genPkgHeader(struct pkg_header *header)
{
	int x = 1;
	int little_endian = (*(char*)&x == (char)1) ? 1 : 0;

	header->little_endian = little_endian;
	header->msg_type = MSG_TYPE_PKG;
	header->data_len = sizeof(struct pkg_data);
}

void genPkgData(struct pkg_data *data, uint32_t idx)
{
	data->idx = idx;

	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	data->sec = (uint64_t)ts.tv_sec;
	data->nsec = (uint64_t)ts.tv_nsec;
}

void sendPkgs(
	muggle_socket_context_t *ctx,
	int flags,
	muggle_benchmark_handle_t *handle,
	muggle_benchmark_config_t *config)
{
	struct pkg msg;
	genPkgHeader(&msg.header);

	struct timespec ts_start, ts_end;
	timespec_get(&ts_start, TIME_UTC);

	muggle_benchmark_record_t *write_beg_records =
		muggle_benchmark_handle_get_records(handle, NET_TRANS_ACTION_WRITE_BEG);
	muggle_benchmark_record_t *write_end_records =
		muggle_benchmark_handle_get_records(handle, NET_TRANS_ACTION_WRITE_END);
	fn_muggle_benchmark_record fn_record = muggle_benchmark_get_fn_record(config->elapsed_unit);

	uint32_t idx = 0;
	for (int i = 0; i < (int)config->rounds; i++)
	{
		for (int j = 0; j < (int)config->record_per_round; j++)
		{
			genPkgData((struct pkg_data*)&msg.placeholder, idx);

			fn_record(&write_beg_records[idx]);
			muggle_socket_ctx_send(
				ctx, &msg, 
				sizeof(struct pkg_header) + (size_t)msg.header.data_len,
				flags);
			fn_record(&write_end_records[idx]);

			++idx;
		}

		if (config->round_interval_ms > 0)
		{
			muggle_msleep(config->round_interval_ms);
		}
	}

	timespec_get(&ts_end, TIME_UTC);
	uint64_t elapsed_ns = (ts_end.tv_sec - ts_start.tv_sec) * 1000000000 + ts_end.tv_nsec - ts_start.tv_nsec;

	MUGGLE_LOG_INFO("send %u pkg completed, use %llu ns", (unsigned int)idx, (unsigned long long)elapsed_ns);

	muggle_msleep(5);

	memset(&msg, 0, sizeof(msg));
	msg.header.msg_type = MSG_TYPE_END;
	muggle_socket_ctx_send(ctx, &msg, sizeof(struct pkg_header) + (size_t)msg.header.data_len, 0);
	MUGGLE_LOG_INFO("send end pkg");
}

int onRecvPkg(
	muggle_socket_context_t *ctx,
	struct pkg *pkg,
	muggle_benchmark_handle_t *handle,
	muggle_benchmark_config_t *config)
{
	// record time
	muggle_benchmark_record_t record;
	fn_muggle_benchmark_record fn_record =
		muggle_benchmark_get_fn_record(config->elapsed_unit);
	fn_record(&record);

	// get records
	muggle_benchmark_record_t *write_beg_records =
		muggle_benchmark_handle_get_records(handle, NET_TRANS_ACTION_WRITE_BEG);
	muggle_benchmark_record_t *read_records =
		muggle_benchmark_handle_get_records(handle, NET_TRANS_ACTION_READ);

	switch (pkg->header.msg_type)
	{
		case MSG_TYPE_PKG:
			{
				struct pkg_data *data = (struct pkg_data*)pkg->placeholder;
				uint32_t idx = data->idx;

				memcpy(&read_records[idx], &record, sizeof(record));

				write_beg_records[idx].idx = idx;
				write_beg_records[idx].ts.tv_sec = data->sec;
				write_beg_records[idx].ts.tv_nsec = data->nsec;
			}break;
		case MSG_TYPE_END:
			{
				MUGGLE_LOG_INFO("recv end message");
				return 1;
			}break;
	}

	return 0;
}

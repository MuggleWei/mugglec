#include "muggle/c/muggle_c.h"
#include "muggle_benchmark/muggle_benchmark.h"

int chan_write(void *user_args, void *data)
{
	return muggle_channel_write((muggle_channel_t*)user_args, data);
}

void* chan_read(void *user_args, int consumer_id)
{
	MUGGLE_UNUSED(consumer_id);

	return muggle_channel_read((muggle_channel_t*)user_args);
}

void producer_complete_cb(muggle_benchmark_config_t *config, void *user_args)
{
	MUGGLE_UNUSED(config);

	static muggle_benchmark_thread_message_t end_msg;
	memset(&end_msg, 0, sizeof(end_msg));
	end_msg.id = UINT64_MAX;
	chan_write(user_args, (void*)&end_msg);
}

void benchmark_chan(muggle_benchmark_config_t *config, int flags, const char *name)
{
	// prepare channel
	muggle_channel_t chan;
	muggle_channel_init(&chan, (muggle_sync_t)config->capacity, flags);

	// initialize thread transfer benchmark
	muggle_benchmark_thread_trans_t benchmark;
	muggle_benchmark_thread_trans_init(
		&benchmark,
		config,
		(void*)&chan,
		chan_write,
		chan_read,
		producer_complete_cb);

	// run benchmark
	muggle_benchmark_thread_trans_run(&benchmark);

	// generate report
	muggle_benchmark_thread_trans_gen_report(&benchmark, name);

	// destroy benchmark
	muggle_benchmark_thread_trans_destroy(&benchmark);

	// free channel
	muggle_channel_destroy(&chan);
}

int main(int argc, char *argv[])
{
	// initialize log
	muggle_log_simple_init(MUGGLE_LOG_LEVEL_INFO, MUGGLE_LOG_LEVEL_INFO);

	// initialize benchmark config
	muggle_benchmark_config_t config;
	muggle_benchmark_config_parse_cli(&config, argc, argv);

	config.producer = 0;

	muggle_benchmark_config_output(&config);

	// channel must guarantee only one reader
	if (config.consumer != 1)
	{
		MUGGLE_LOG_ERROR(
			"user must guarantee only one reader consume message from channel");
		exit(EXIT_FAILURE);
	}

	int flags = 0;

	int hc = (int)muggle_thread_hardware_concurrency();
	if (hc <= 0)
	{
		hc = 2;
	}

	int producer_nums[] = {
		1, 2, 4, hc / 2, hc, hc * 2, hc * 4
	};

	int w_flags[] = {
		MUGGLE_CHANNEL_FLAG_WRITE_SYNC,
		MUGGLE_CHANNEL_FLAG_WRITE_MUTEX,
		MUGGLE_CHANNEL_FLAG_WRITE_SPIN,
		MUGGLE_CHANNEL_FLAG_WRITE_SINGLE,
	};

	int r_flags[] = {
		MUGGLE_CHANNEL_FLAG_READ_SYNC,
		MUGGLE_CHANNEL_FLAG_READ_MUTEX,
		MUGGLE_CHANNEL_FLAG_READ_BUSY,
	};

	const char *str_w_flags = NULL;
	const char *str_r_flags = NULL;
	char name[64];
	for (int i = 0; i < (int)(sizeof(producer_nums) / sizeof(producer_nums[0])); i++)
	{
		for (int w = 0; w < (int)(sizeof(w_flags) / sizeof(w_flags[0])); w++)
		{
			for (int r = 0; r < (int)(sizeof(r_flags) / sizeof(r_flags[0])); r++)
			{
				int wflag = w_flags[w];
				int rflag = r_flags[r];
				int num_producer = producer_nums[i];

				if (wflag == MUGGLE_CHANNEL_FLAG_WRITE_SINGLE && num_producer != 1)
				{
					continue;
				}

				switch (wflag)
				{
				case MUGGLE_CHANNEL_FLAG_WRITE_SYNC:
					{
						str_w_flags = "sync";
					}break;
				case MUGGLE_CHANNEL_FLAG_WRITE_MUTEX:
					{
						str_w_flags = "mutex";
					}break;
				case MUGGLE_CHANNEL_FLAG_WRITE_SPIN:
					{
						str_w_flags = "spin";
					}break;
				case MUGGLE_CHANNEL_FLAG_WRITE_SINGLE:
					{
						str_w_flags = "single";
					}break;
				default:
					{
						MUGGLE_LOG_ERROR("unrecognized write flags: %d", w);
						exit(EXIT_FAILURE);
					}break;
				}

				switch (rflag)
				{
				case MUGGLE_CHANNEL_FLAG_READ_SYNC:
					{
						str_r_flags = "sync";
					}break;
				case MUGGLE_CHANNEL_FLAG_READ_MUTEX:
					{
						str_r_flags = "mutex";
					}break;
				case MUGGLE_CHANNEL_FLAG_READ_BUSY:
					{
						str_r_flags = "busy";
					}break;
				default:
					{
						MUGGLE_LOG_ERROR("unrecognized read flags: %d", w);
						exit(EXIT_FAILURE);
					}break;
				}

				flags = wflag | rflag;
				memset(name, 0, sizeof(name));
				snprintf(name, sizeof(name), "channel_%d_%s_w_%s_r",
					num_producer, str_w_flags, str_r_flags);

				config.producer = num_producer;

				MUGGLE_LOG_INFO("--------------------------------------------------------");
				MUGGLE_LOG_INFO("run channel - %d %s write and %s read",
					num_producer, str_w_flags, str_r_flags);
				benchmark_chan(&config, flags, name);
			}
		}
	}

	return 0;
}

#include "muggle_benchmark_handle.h"

int muggle_benchmark_handle_init(
	muggle_benchmark_handle_t *handle,
	uint64_t record_count,
	int action_count)
{
	memset(handle, 0, sizeof(*handle));

	handle->record_count = record_count;
	handle->action_count = action_count;
	handle->action_name = (char**)malloc(sizeof(char*) * action_count);
	if (handle->action_name == NULL)
	{
		goto benchmark_handle_init_except;
	}
	for (int i = 0; i < handle->action_count; i++)
	{
		handle->action_name[i] = NULL;
	}

	handle->action_records = (muggle_benchmark_record_t**)malloc(sizeof(muggle_benchmark_record_t*) * action_count);
	if (handle->action_records == NULL)
	{
		goto benchmark_handle_init_except;
	}
	for (int i = 0; i < handle->action_count; i++)
	{
		handle->action_records[i] = NULL;
	}

	return 0;

benchmark_handle_init_except:
	muggle_benchmark_handle_destroy(handle);
	return -1;
}

void muggle_benchmark_handle_destroy(muggle_benchmark_handle_t *handle)
{
	if (handle->action_name)
	{
		for (int i = 0; i < handle->action_count; i++)
		{
			if (handle->action_name[i])
			{
				free(handle->action_name[i]);
				handle->action_name[i] = NULL;
			}
		}
		free(handle->action_name);
		handle->action_name = NULL;
	}

	if (handle->action_records)
	{
		for (int i = 0; i < handle->action_count; i++)
		{
			if (handle->action_records[i])
			{
				free(handle->action_records[i]);
				handle->action_records[i] = NULL;
			}
		}
		free(handle->action_records);
		handle->action_records = NULL;
	}
}

int muggle_benchmark_handle_set_action(muggle_benchmark_handle_t *handle, int action, const char *action_name)
{
	if (action < 0 || action >= handle->action_count)
	{
		MUGGLE_LOG_ERROR("invalid action value: %d", action);
		return -1;
	}

	if (handle->action_name[action])
	{
		MUGGLE_LOG_ERROR("repeated set benchmark handle action: %d", action);
		return -1;
	}

	size_t len_action_name = strlen(action_name);
	handle->action_name[action] = (char*)malloc(sizeof(char) * len_action_name + 1);
	if (!handle->action_name[action])
	{
		goto benchmark_handle_set_action_except;
	}
	strncpy(handle->action_name[action], action_name, strlen(action_name));
	handle->action_name[action][len_action_name] = '\0';

	handle->action_records[action] =
		(muggle_benchmark_record_t*)malloc(sizeof(muggle_benchmark_record_t) * handle->record_count);
	if (!handle->action_records[action])
	{
		goto benchmark_handle_set_action_except;
	}
	for (uint64_t i = 0; i < handle->record_count; i++)
	{
		memset(&handle->action_records[action][i], 0, sizeof(muggle_benchmark_record_t));
		handle->action_records[action][i].idx = i;
		handle->action_records[action][i].action = action;
	}

	return 0;

benchmark_handle_set_action_except:
	if (handle->action_name[action])
	{
		free(handle->action_name[action]);
		handle->action_name[action] = NULL;
	}

	return -1;
}

uint64_t muggle_benchmark_handle_get_record_count(muggle_benchmark_handle_t *handle)
{
	return handle->record_count;
}

const char* muggle_benchmark_handle_get_action_name(muggle_benchmark_handle_t *handle, int action)
{
	if (action < 0 || action >= handle->action_count)
	{
		return NULL;
	}

	return handle->action_name[action];
}

muggle_benchmark_record_t* muggle_benchmark_handle_get_records(muggle_benchmark_handle_t *handle, int action)
{
	if (action < 0 || action >= handle->action_count)
	{
		return NULL;
	}

	return handle->action_records[action];
}

void muggle_benchmark_handle_gen_records_report(
	muggle_benchmark_handle_t *handle,
	FILE *fp,
	int elapsed_unit)
{
	for (uint64_t i = 0; i < handle->record_count; i++)
	{
		for (int action = 0; action < handle->action_count; action++)
		{
			const char *action_name = muggle_benchmark_handle_get_action_name(handle, action);
			if (action_name == NULL)
			{
				continue;
			}

			muggle_benchmark_record_t *records = muggle_benchmark_handle_get_records(handle, action);
			muggle_benchmark_record_t *record = records + i;
			if (elapsed_unit == MUGGLE_BENCHMARK_ELAPSED_UNIT_NS)
			{
				fprintf(fp, "%llu,%s,%llu,%lu\n",
					(unsigned long long)record->idx,
					action_name,
					(unsigned long long)record->ts.tv_sec,
					(unsigned long)record->ts.tv_nsec);
			}
			else if (elapsed_unit == MUGGLE_BENCHMARK_ELAPSED_UNIT_CPU_CYCLE)
			{
				fprintf(fp, "%llu,%s,%llu\n",
					(unsigned long long)record->idx,
					action_name,
					(unsigned long long)record->cpu_cycles);
			}
		}
	}
}

void muggle_benchmark_gen_latency_report_head(FILE *fp, struct muggle_benchmark_config *config)
{
	char buf[4096] = {0};
	if (config->elapsed_unit == MUGGLE_BENCHMARK_ELAPSED_UNIT_NS)
	{
		fprintf(fp, "elapsed unit[ns]\n");
	}
	else if (config->elapsed_unit == MUGGLE_BENCHMARK_ELAPSED_UNIT_CPU_CYCLE)
	{
		fprintf(fp, "elapsed unit[cpu cycle]\n");
	}

	fprintf(fp, "case_name,rounds,record_per_round,interval_ms,capacity,producer,consumer,avg,");
	for (int i = 0; i < 100; i += config->report_step)
	{
		fprintf(fp, "%d,", i);
	}
	fprintf(fp, "100\n");
}

static int compare_uint64(const void *a, const void *b)
{
	const uint64_t arg1 = *(const uint64_t*)a;
	const uint64_t arg2 = *(const uint64_t*)b;

	if (arg1 < arg2) return -1;
	if (arg1 > arg2) return 1;
	return 0;
}

uint64_t muggle_benchmark_get_elapsed_ns(
	muggle_benchmark_record_t *r1, muggle_benchmark_record_t *r2)
{
	return
		(uint64_t)(r2->ts.tv_sec - r1->ts.tv_sec) * 1000000000
		+ (uint64_t)r2->ts.tv_nsec
		- (uint64_t)r1->ts.tv_nsec;
}

uint64_t muggle_benchmark_get_elapsed_cpu_cycles(
	muggle_benchmark_record_t *r1, muggle_benchmark_record_t *r2)
{
	return r2->cpu_cycles - r1->cpu_cycles;
}

void muggle_benchmark_gen_latency_report_body(
	FILE *fp,
	muggle_benchmark_handle_t *handle,
	struct muggle_benchmark_config *config,
	int action1,
	int action2,
	bool sort_result)
{
	const char *action_name1 = muggle_benchmark_handle_get_action_name(handle, action1);
	const char *action_name2 = muggle_benchmark_handle_get_action_name(handle, action2);
	muggle_benchmark_record_t *rs1 = muggle_benchmark_handle_get_records(handle, action1);
	muggle_benchmark_record_t *rs2 = muggle_benchmark_handle_get_records(handle, action2);

	uint64_t *elapseds = (uint64_t*)malloc(handle->record_count * sizeof(uint64_t));
	uint64_t sum = 0, cnt_sum = 0, avg = 0;
	for (uint64_t i = 0; i < handle->record_count; ++i)
	{
		muggle_benchmark_record_t *r1 = rs1 + i;
		muggle_benchmark_record_t *r2 = rs2 + i;

		if (config->elapsed_unit == MUGGLE_BENCHMARK_ELAPSED_UNIT_NS)
		{
			if (r1->ts.tv_sec == 0 || r2->ts.tv_sec == 0)
			{
				elapseds[i] = UINT_MAX;
			}
			else
			{
				elapseds[i] = muggle_benchmark_get_elapsed_ns(r1, r2);
				sum += elapseds[i];
				cnt_sum++;
			}
		}
		else if (config->elapsed_unit == MUGGLE_BENCHMARK_ELAPSED_UNIT_CPU_CYCLE)
		{
			if (r1->cpu_cycles == 0 || r2->cpu_cycles == 0)
			{
				elapseds[i] = UINT_MAX;
			}
			else
			{
				elapseds[i] = muggle_benchmark_get_elapsed_cpu_cycles(r1, r2);
				sum += elapseds[i];
				cnt_sum++;
			}
		}
		else
		{
			MUGGLE_LOG_ERROR("invalid elapseds unit");
			exit(EXIT_FAILURE);
		}
	}
	avg = sum / cnt_sum;

	if (sort_result)
	{
		qsort(elapseds, handle->record_count, sizeof(uint64_t), compare_uint64);
	}

	// case name
	fprintf(fp, "%s->%s by %s,",
		action_name1, action_name2, 
		sort_result ? "elapsed" : "idx");

	// rounds,record_per_round,interval_ms,capacity,producer,consumer,
	fprintf(fp, "%llu,%llu,%lld,%d,%d,%d,",
		(unsigned long long)config->rounds,
		(unsigned long long)config->record_per_round,
		(long long)config->round_interval_ns,
		config->capacity,
		config->producer,
		config->consumer);

	fprintf(fp, "%llu,", (unsigned long long)avg);

	for (int i = 0; i < 100; i += config->report_step)
	{
		uint64_t idx = (uint64_t)((i / 100.0) * handle->record_count);
		if (elapseds[idx] == UINT_MAX)
		{
			fprintf(fp, "-,");
		}
		else
		{
			fprintf(fp, "%llu,", (unsigned long long)elapseds[idx]);
		}
	}

	if (elapseds[handle->record_count-1] == UINT_MAX)
	{
		fprintf(fp, "-\n");
	}
	else
	{
		fprintf(fp, "%llu\n", (unsigned long long)elapseds[handle->record_count-1]);
	}

	free(elapseds);
}

static void muggle_benchmark_record_ns(muggle_benchmark_record_t *record)
{
	muggle_realtime_get(record->ts);
}
static void muggle_benchmark_record_cpu_cycle(muggle_benchmark_record_t *record)
{
	record->cpu_cycles = muggle_get_cpu_cycle();
}

fn_muggle_benchmark_record muggle_benchmark_get_fn_record(int elapsed_unit)
{
	if (elapsed_unit == MUGGLE_BENCHMARK_ELAPSED_UNIT_CPU_CYCLE)
	{
		return muggle_benchmark_record_cpu_cycle;
	}
	else
	{
		return muggle_benchmark_record_ns;
	}
}

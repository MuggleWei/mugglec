/*
 *	author: muggle wei <mugglewei@gmail.com>
 *
 *	Use of this source code is governed by the MIT license that can be
 *	found in the LICENSE file.
 */

#include "muggle_benchmark.h"

NS_MUGGLE_BEGIN

void muggle_benchmark_gen_records_report(
	FILE *fp,
	const char *action_name[],
	int elapsed_unit,
	int cnt_array,
	int cnt_record,
	muggle_benchmark_record_t *records[])
{
	for (int i = 0; i < cnt_record; i++)
	{
		for (int j = 0; j < cnt_array; j++)
		{
			muggle_benchmark_record_t *record = &records[j][i];
			if (elapsed_unit == MUGGLE_BENCHMARK_ELAPSED_UNIT_NS)
			{
				fprintf(fp, "%llu,%s,%llu,%lu\n",
					(unsigned long long)record->idx,
					action_name[record->action],
					(unsigned long long)record->ts.tv_sec,
					(unsigned long)record->ts.tv_nsec);
			}
			else if (elapsed_unit == MUGGLE_BENCHMARK_ELAPSED_UNIT_CPU_CYCLE)
			{
				fprintf(fp, "%llu,%s,%llu\n",
					(unsigned long long)record->idx,
					action_name[record->action],
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

	fprintf(fp, "case_name,producer,consumer,rounds,interval_ms,record_per_round,avg,");
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

void muggle_benchmark_gen_latency_report_body(
	FILE *fp,
	const char *action_name[],
	struct muggle_benchmark_config *config,
	uint64_t cnt_record,
	muggle_benchmark_record_t *rs1,
	muggle_benchmark_record_t *rs2,
	bool sort_result)
{
	uint64_t *elapseds = (uint64_t*)malloc(cnt_record * sizeof(uint64_t));
	uint64_t sum = 0, cnt_sum = 0, avg = 0;
	for (uint64_t i = 0; i < cnt_record; ++i)
	{
		muggle_benchmark_record_t *r1 = &rs1[i];
		muggle_benchmark_record_t *r2 = &rs2[i];

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
		qsort(elapseds, cnt_record, sizeof(uint64_t), compare_uint64);
	}

	// case name
	fprintf(fp, "%s->%s by %s,",
		action_name[rs1[0].action], action_name[rs2[0].action], 
		sort_result ? "elapsed" : "idx");

	// rounds,record_per_round,interval_ms,capacity,producer,consumer,
	fprintf(fp, "%llu,%llu,%d,%d,%d,%d,",
		(unsigned long long)config->rounds,
		(unsigned long long)config->record_per_round,
		config->round_interval_ms,
		config->capacity,
		config->producer,
		config->consumer);


	for (int i = 0; i < 100; i += config->report_step)
	{
		uint64_t idx = (uint64_t)((i / 100.0) * cnt_record);
		if (elapseds[idx] == UINT_MAX)
		{
			fprintf(fp, "-,");
		}
		else
		{
			fprintf(fp, "%llu,", (unsigned long long)elapseds[idx]);
		}
	}

	if (elapseds[cnt_record-1] == UINT_MAX)
	{
		fprintf(fp, "-\n");
	}
	else
	{
		fprintf(fp, "%llu\n", (unsigned long long)elapseds[cnt_record-1]);
	}

	free(elapseds);
}

uint64_t muggle_benchmark_get_elapsed_ns(
	muggle_benchmark_record_t *r1, muggle_benchmark_record_t *r2)
{
	return
		(uint64_t)(r2->ts.tv_sec - r1->ts.tv_sec) * 1000000000
		+ (uint64_t)r2->ts.tv_nsec
		- (uint64_t)r1->ts.tv_nsec;
}

uint64_t get_elapsed_cpu_cycles(
	muggle_benchmark_record_t *r1, muggle_benchmark_record_t *r2)
{
	return r2->cpu_cycles - r1->cpu_cycles;
}

NS_MUGGLE_END

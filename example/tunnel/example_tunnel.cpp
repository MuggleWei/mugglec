#include "muggle/cpp/tunnel/tunnel.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <queue>

struct TunnelMessage
{
	int id;
	int msg_id;
	std::chrono::time_point<std::chrono::high_resolution_clock> tp;
};

void productor(int id, int task_num, muggle::Tunnel<TunnelMessage> &tunnel)
{
	for (int i = 0; i < task_num; ++i)
	{
		TunnelMessage msg{
			id, i, std::chrono::high_resolution_clock::now()
		};
		tunnel.Write(std::move(msg));
	}
}

int consumer(int id, muggle::Tunnel<TunnelMessage> &tunnel)
{
	int cnt = 0;
	while (true)
	{
		std::queue<TunnelMessage> msg_queue;
		int ret = tunnel.Read(msg_queue);
		if (ret == muggle::TUNNEL_RETURN_TYPE::TUNNEL_CLOSED)
		{
			break;
		}

		while (msg_queue.size() > 0)
		{
			msg_queue.pop();
			++cnt;
		}
	}

	return cnt;
}

void run(int num_productor, int num_consumer, int task_num_per_productor)
{
	std::vector<std::thread> consumer_threads;
	std::vector<std::thread> productor_threads;
	muggle::Tunnel<TunnelMessage> tunnel;
	muggle::Tunnel<std::string> log_tunnel;

	for (int i = 0; i < num_consumer; ++i)
	{
		consumer_threads.emplace_back([&tunnel, &log_tunnel, i] {
			int cnt = consumer(i, tunnel);

			char buf_log[128];
			snprintf(buf_log, sizeof(buf_log), "consumer #%d consumed: %d", i, cnt);
			log_tunnel.Write(std::string(buf_log));
		});
	}

	auto total_start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < num_productor; ++i)
	{
		productor_threads.emplace_back([&tunnel, &log_tunnel, i, task_num_per_productor] {
			auto start = std::chrono::high_resolution_clock::now();

			productor(i, task_num_per_productor, tunnel);

			auto end = std::chrono::high_resolution_clock::now();
			auto elapsed = end - start;
			auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

			char buf_log[128];
			snprintf(buf_log, sizeof(buf_log),
				"productor #%d producted %d use %I64d ms",
				i, task_num_per_productor, ms);
			log_tunnel.Write(std::string(buf_log));
		});
	}

	for (int i = 0; i < num_productor; ++i)
	{
		productor_threads[i].join();
	}

	auto total_end = std::chrono::high_resolution_clock::now();
	auto total_elapsed = total_end - total_start;
	auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(total_elapsed).count();

	tunnel.Close();

	for (int i = 0; i < num_consumer; ++i)
	{
		consumer_threads[i].join();
	}

	std::queue<std::string> log_queue;
	log_tunnel.Read(log_queue);
	while (log_queue.size() > 0)
	{
		std::string &log = log_queue.front();
		std::cout << log << std::endl;
		log_queue.pop();
	}
	std::cout << "mission completed: "
		<< num_productor << " productors, "
		<< num_consumer << " consumer, "
		<< task_num_per_productor << " tasks per productor,"
		<< "use time: " << total_ms << "ms" << std::endl;
}

void exampleSingleConsumer(int task_num_per_productor)
{
	auto n = std::thread::hardware_concurrency();
	n = n > 2 ? n : 2;
	run(n, 1, task_num_per_productor);
}

void exampleMultipleConsumer(int task_num_per_productor)
{
	auto n = std::thread::hardware_concurrency();
	n = n > 2 ? n : 2;
	run(n, n, task_num_per_productor);
}

int main()
{
	int task_num_per_productor = 10000;

	exampleSingleConsumer(task_num_per_productor);
	exampleMultipleConsumer(task_num_per_productor);

	return 0;
}
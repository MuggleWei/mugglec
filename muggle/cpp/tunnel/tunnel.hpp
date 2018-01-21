#ifndef MUGGLE_TUNNEL_H_
#define MUGGLE_TUNNEL_H_

#include "muggle/c/base/macro.h"
#include <queue>
#include <mutex>
#include <condition_variable>

NS_MUGGLE_BEGIN

enum TUNNEL_RETURN_TYPE
{
	TUNNEL_SUCCESS = 0,
	TUNNEL_CLOSED, // tunnel already closed
	TUNNEL_FULLED, // tunnel fulled
};

template<typename T>
class Tunnel
{
public:
	Tunnel(unsigned int max_size = 0)
		: max_size_(max_size)
		, is_closed_(false)
	{}
	~Tunnel() {}

	Tunnel(const Tunnel &rhs) = delete;
	Tunnel(Tunnel &&lhs) = delete;
	Tunnel& operator=(const Tunnel &rhs) = delete;
	Tunnel& operator=(Tunnel &&lhs) = delete;

	int Write(const T &val)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		if (is_closed_)
		{
			return TUNNEL_RETURN_TYPE::TUNNEL_CLOSED;
		}
		if (max_size_ != 0 && queue_.size() >= max_size_)
		{
			return TUNNEL_RETURN_TYPE::TUNNEL_FULLED;
		}
		queue_.push(val);
		cv_.notify_one();

		return TUNNEL_RETURN_TYPE::TUNNEL_SUCCESS;
	}

	int Write(T &&val)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		if (is_closed_)
		{
			return TUNNEL_RETURN_TYPE::TUNNEL_CLOSED;
		}
		if (max_size_ != 0 && (decltype(max_size_))queue_.size() >= max_size_)
		{
			return TUNNEL_RETURN_TYPE::TUNNEL_FULLED;
		}
		queue_.push(std::move(val));
		cv_.notify_one();

		return TUNNEL_RETURN_TYPE::TUNNEL_SUCCESS;
	}

	int Read(std::queue<T> &ret_queue)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		while (queue_.size() == 0 && !is_closed_)
		{
			cv_.wait(lock);
		}

		if (is_closed_)
		{
			if (queue_.size() != 0)
			{
				std::swap(queue_, ret_queue);
				return TUNNEL_RETURN_TYPE::TUNNEL_SUCCESS;
			}
			return TUNNEL_RETURN_TYPE::TUNNEL_CLOSED;
		}

		std::swap(queue_, ret_queue);
		return TUNNEL_RETURN_TYPE::TUNNEL_SUCCESS;
	}

	void Close()
	{
		std::unique_lock<std::mutex> lock(mtx_);
		is_closed_ = true;
		cv_.notify_all();
	}

private:
	std::queue<T> queue_;
	std::mutex mtx_;
	std::condition_variable cv_;

	int max_size_;
	bool is_closed_;
};

NS_MUGGLE_END

#endif

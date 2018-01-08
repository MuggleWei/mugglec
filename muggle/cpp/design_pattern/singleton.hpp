#ifndef MUGGLE_SINGLETON_H_
#define MUGGLE_SINGLETON_H_

#include <thread>
#include <mutex>
#include "muggle/c/base/macro.h"

NS_MUGGLE_BEGIN

template<typename T>
class Singleton
{
public:
	template<typename... Args>
	static T& Instance(Args&&... args)
	{
		if (singleton_ == nullptr)
		{
			std::lock_guard<std::mutex> lock(mtx_);
			if (singleton_ == nullptr)
			{
				singleton_ = new T(std::forward<Args>(args)...);
				atexit(Destroy);
			}
		}

		return *singleton_;
	}

protected:
	Singleton() {}
	virtual ~Singleton() {}

private:
	Singleton(const Singleton& rhs) {}
	Singleton& operator=(const Singleton& rhs) {}

	static void Destroy()
	{
		if (singleton_ != nullptr)
		{
			delete singleton_;
			singleton_ = nullptr;
		}
	}

private:
	static T* volatile singleton_;
	static std::mutex mtx_;
};

template<typename T>
T* volatile Singleton<T>::singleton_ = nullptr;
template<typename T>
std::mutex Singleton<T>::mtx_;

NS_MUGGLE_END

#endif
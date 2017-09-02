#ifndef MUGGLE_SINGLETON_H_
#define MUGGLE_SINGLETON_H_

#include "muggle/base_c/macro.h"
#include "muggle/utils_cpp/utils_cpp_macro.h"
#include "muggle/base_cpp/lock.h"

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
			ScopeLock lock(mutex_);
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
	static MutexLock mutex_;
};

template<typename T>
T* volatile Singleton<T>::singleton_ = nullptr;
template<typename T>
MutexLock Singleton<T>::mutex_;

NS_MUGGLE_END

#endif
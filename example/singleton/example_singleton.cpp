#include <iostream>
#include "muggle/cpp/design_pattern/singleton.hpp"

class A
{
public:
	A()
		: i_(0)
	{
		std::cout << "A()" << std::endl;
	}
	A(int i)
		: i_(i)
	{
		std::cout << "A(i)" << std::endl;
	}

	void print()
	{
		std::cout << i_ << std::endl;
	}

private:
	int i_;
};

class ResManager : public muggle::Singleton<ResManager>
{
public:
	ResManager(int i = 0)
		: res_(i)
	{}
	void Init(int res)
	{
		res_ = res;
	}
	void print()
	{
		std::cout << res_ << std::endl;
	}

private:
	int res_;
};

int main(int argc, char *argv[])
{
	muggle::Singleton<A>::Instance(5);
	muggle::Singleton<A>::Instance().print();

	ResManager::Instance(1);
	ResManager::Instance().Init(5);
	ResManager::Instance().print();

	return 0;
}
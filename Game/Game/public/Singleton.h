#pragma once
#include <type_traits>
template <typename T>
class Singleton
{
public:
	template<typename... Args>
	static T& instance(Args&&... args)
	{
		static T t(std::forward<Args>(args)...);
		return t;
	}
	Singleton() {}
	virtual ~Singleton() {}
	Singleton(const Singleton&) = delete;
	Singleton& operator = (const Singleton&) = delete;
};
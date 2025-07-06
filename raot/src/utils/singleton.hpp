#pragma once
#include "structs.hpp"
//单例模式模板类
template <typename T>
class singleton {
protected:
	DEFAULT_CTOR_DTOR(singleton);
	NON_COPYABLE(singleton);

public:
	[[nodiscard]] static T& get() {
		static T instance = {};
		return instance;
	}
};
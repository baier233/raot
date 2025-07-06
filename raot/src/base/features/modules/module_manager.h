#pragma once

#include "abstract_module.h"
#include <event_bus.hpp>
#define ToBaseModule(__MOD) reinterpret_cast<abstract_module*>(__MOD)
#define ToDirectModule(__Impl, __Mod) reinterpret_cast<__Impl*>(__Mod)

typedef void* HMOD;

class module_manager
{
private:
	std::vector<HMOD> modules;
	std::vector<HMOD> enable_modules;
public:
	module_manager();

	~module_manager() = default;

	template<class MODCLASS>
	void add_module();

	std::vector<dxg::handler_registration> registrations;

	template<typename MODCLASS>
	MODCLASS& get_module();
	inline void get_module(category c, std::vector<HMOD>* out_module) {
		out_module->clear();

		for (auto iter = this->modules.cbegin(); iter < this->modules.cend(); iter++) {
			if (ToBaseModule(*iter)->get_category() == c) {
				out_module->push_back(*iter);
			}
		}
	}

	abstract_module* get_module(const char* name);

	void get_modules(category c, std::vector<HMOD>& out_module);

	void get_modules(bool isenable, std::vector<HMOD>& out_module);

	module_manager(module_manager&&) = delete;

	module_manager(const module_manager&) = delete;

	module_manager& operator=(const module_manager&) = delete;
	std::vector<HMOD> get_modules();
	std::vector<HMOD> get_enable_modules();
	bool load_modules();

	inline void clean() { modules.clear(); }
};


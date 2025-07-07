#include "module_manager.h"


module_manager::module_manager()
{
}

abstract_module* module_manager::get_module(const char* name)
{
	for (auto mod : this->modules)
	{
		if (ToBaseModule(mod)->get_name() == name)
		{
			return ToBaseModule(mod);
		}
	}
	return nullptr;
}

void module_manager::get_modules(category c, std::vector<HMOD>& out_module)
{
	for (auto mod : this->modules)
	{
		if (ToBaseModule(mod)->get_category() == c)
		{
			out_module.push_back(mod);
		}
	}
}

void module_manager::get_modules(bool isenable, std::vector<HMOD>& out_module)
{
	for (auto& mod : this->modules)
	{
		if (ToBaseModule(mod)->get_toggle() == isenable)
		{
			out_module.push_back(mod);
		}
	}
}

template<class MODCLASS>
void module_manager::add_module()
{
	this->modules.push_back(reinterpret_cast<HMOD>(&instance_module<MODCLASS>::instance()));
}

template<typename MODCLASS>
MODCLASS& module_manager::get_module()
{
	return instance_module<MODCLASS>::instance();
}


std::vector<HMOD> module_manager::get_modules()
{
	return this->modules;
}
std::vector<HMOD> module_manager::get_enable_modules()
{
	return this->enable_modules;
}
#include "impl/render/nametag.h"
bool module_manager::load_modules()
{
	this->add_module<nametag>();
	return true;
}
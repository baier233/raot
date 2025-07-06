#include "abstract_module.h"

category abstract_module::get_category()
{
	return this->_category;
}

void abstract_module::toggle()
{
	this->on_toggle();
}

void abstract_module::on_toggle()
{
	if (this->i_toggle) this->on_disable();
	if (!this->i_toggle) this->on_enable();
	this->i_toggle = !this->i_toggle;
}

std::string abstract_module::get_name()
{
	const auto& input = this->name;
	if (input.empty()) {
		return input;
	}

	std::string result;
	bool capitalizeNext = false;

	// 处理第一个字符
	result += toupper(input[0]);

	// 处理剩余字符
	for (size_t i = 1; i < input.size(); ++i) {
		if (input[i] == '_') {
			capitalizeNext = true;
		}
		else {
			if (capitalizeNext) {
				result += toupper(input[i]);
				capitalizeNext = false;
			}
			else {
				result += input[i];
			}
		}
	}

	return result;
}

std::string abstract_module::get_desc()
{
	return this->desc;
}

int abstract_module::get_key()
{
	return this->key;
}

bool abstract_module::get_toggle()
{
	return this->i_toggle;
}

void abstract_module::set_toggle(bool _new)
{
	if (_new == this->i_toggle) return;
	this->toggle();
}

void abstract_module::set_key(int _new)
{
	this->key = _new;
}

abstract_module::abstract_module(const char* name, category cate) : name(name), _category(cate) {}

abstract_module::abstract_module(const char* name, category cate, std::string desc) : name(name), _category(cate), desc(desc) {}

abstract_module::abstract_module(const char* name, category cate, int k) : name(name), _category(cate), key(k) {}

abstract_module::abstract_module(const char* name, category cate, std::string desc, int k) : name(name), _category(cate), desc(desc), key(k) {}
#include <base/raot.h>
void abstract_module::set_toggle_async(bool _new)
{
	std::unique_lock<std::mutex> lock(raot::get().module_events_mutex);
	if (_new == this->i_toggle) return;
	raot::get().module_events.push_back({ this });
}

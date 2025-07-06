#pragma once
#include <utils/singleton.hpp>
#include <base/features/modules/module_manager.h>
#include <deque>
#include <mutex>
#include <event_bus.hpp>

struct key_event
{
	int key_code = 0;
};
#include <base/features/modules/abstract_module.h>
struct module_event
{
	abstract_module* module;
};
class raot : public singleton<raot>
{
public:
	std::mutex key_events_mutex;
	std::mutex module_events_mutex;
	std::vector<key_event> key_events;
	std::vector<module_event> module_events;
	dxg::event_bus event_bus;
	module_manager* mm;
	bool exited = false;
	bool setup();
	void enter_loop();
	bool exit() const;
};


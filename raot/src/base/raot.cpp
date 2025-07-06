#include "raot.h"



#include <base/webui/server.h>
#include <main.hpp>
#include <base/features/modules/common_data.h>

#include <windows.h>

bool raot::setup()
{
	bool flag = false;

	static auto _mm = std::make_unique<module_manager>();

	mm = _mm.get();
	(void)common_data::get();
	mm->load_modules();
	webui::server::get().start(8080);
	return flag;
}
void raot::enter_loop()
{


	while (!this->exited) {

		auto mods = mm->get_modules();

		std::vector<key_event> key_to_process;

		{
			std::lock_guard<std::mutex> lock(key_events_mutex);
			key_to_process = key_events;
			key_events.clear();
		}
		std::vector<module_event> module_to_process;
		{
			std::lock_guard<std::mutex> lock(module_events_mutex);
			module_to_process = module_events;
			module_events.clear();
		}

		for (const auto& event : key_to_process) {
			for (auto& m : mods) {
				auto module = reinterpret_cast<abstract_module*>(m);

				if (module->get_key() == event.key_code) {
					module->set_toggle(!module->get_toggle());
				}
			}
		}
		for (const auto& event : module_to_process)
		{
			event.module->toggle();
		}

		for (auto& m : mods)
		{
			reinterpret_cast<abstract_module*>(m)->on_update();
		}



		Sleep(1);
	}


	this->exit();

}
#include "cleaner/unloaded_module.h"
bool raot::exit() const
{
	auto mods = mm->get_modules();
	for (auto& m : mods) {
		auto module = reinterpret_cast<abstract_module*>(m);
		if (module->get_toggle()) {
			module->set_toggle(false);
		}
	}

	cleaner::unloaded_module::clean();
	webui::server::get().detach();
	FreeLibraryAndExitThread(main::current_module, 6);

	return exited;
}

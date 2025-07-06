#include "config_system.h"
#include <filesystem>
#include <Windows.h>
#include <json.hpp>
#include <base/raot.h>
void config_system::load_config(const std::string& name)
{
	std::filesystem::path jsonFilePath = this->get_config_path() / name;
	nlohmann::json jsonObject;
	try
	{
		std::ifstream jsonFile(jsonFilePath);
		jsonFile >> jsonObject;
		jsonFile.close();
	}
	catch (const std::exception& e)
	{
		return;
	}
	try
	{
		for (const auto& [name, config] : jsonObject.items())
		{
			auto* mod = ToBaseModule(raot::get().mm->get_module(name.c_str()));
			if (mod)
			{
				auto should_toggle = config["Enabled"].get<bool>();
				if (mod->get_toggle() != should_toggle)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					mod->set_toggle_async(should_toggle);
				}
				mod->set_key(config["key"]);

				for (const auto& cv : config["values"])
				{

					for (const auto& [type, value] : mod->get_values())
					{
						auto jValueName = cv["name"].get<std::string>();
						auto valueName = value->get_name();
						if (jValueName == valueName)
						{
							switch (type)
							{
							case values::BOOL:
							{
								auto* pValue = dynamic_cast<values::bool_value*>(value.get());
								*pValue->get_value_ptr() = cv["value"].get<bool>();
								break;
							}
							case values::FLOAT:
							{
								values::float_value* pValue = dynamic_cast<values::float_value*>(value.get());
								*pValue->get_value_ptr() = cv["value"].get<float>();
								break;
							}
							case values::LIST:
							{
								{
									values::mode_value* pValue = dynamic_cast<values::mode_value*>(value.get());
									const std::string& modeDesc = cv["value"].get<std::string>();
									for (size_t i = 0; i < pValue->get_modes().size(); i++)
									{
										if (pValue->get_descs()[i] == modeDesc)
										{
											*pValue->get_value_ptr() = i;
											break;
										}
									}
								}
							}
							break;
							default:
								break;
							}
						}
					}
				}
			}
		}
	}
	catch (const std::exception& e)
	{
	}




}

void config_system::save_config(const std::string& name)
{
	nlohmann::json jsonObject;

	for (void* mods : raot::get().mm->get_modules())
	{
		nlohmann::json config;
		auto* mod = ToBaseModule(mods);
		config["Enabled"] = mod->get_toggle();
		config["key"] = mod->get_key();
		if (!mod->get_values().empty()) {
			for (const auto& v : mod->get_values()) {
				nlohmann::json moduleSet;
				moduleSet["name"] = v.second->get_name();
				auto& value = v.second;
				switch (v.first) {
				case values::BOOL:
					if (auto boolValue = dynamic_cast<values::bool_value*>(value.get())) {
						moduleSet["value"] = boolValue->get_value();
					}
					break;
				case values::INT:
					if (auto intValue = dynamic_cast<values::number_value*>(value.get())) {
						moduleSet["value"] = intValue->get_value();
					}
					break;
				case values::FLOAT:
					if (auto floatValue = dynamic_cast<values::float_value*>(value.get())) {
						moduleSet["value"] = floatValue->get_value();
					}
					break;
				case values::LIST:
					if (auto listValue = dynamic_cast<values::mode_value*> (value.get())) {
						moduleSet["value"] = listValue->get_descs()[listValue->get_value()];
					}
					break;
				default:
					break;
				}
				if (!moduleSet.empty() && !moduleSet.is_null())config["values"].push_back(moduleSet);
			}
		}
		jsonObject[mod->get_name()] = config;
	}
	std::filesystem::path jsonFilePath = this->get_config_path() / name;

	try
	{
		std::ofstream jsonFile(jsonFilePath);
		jsonFile << jsonObject.dump(4); // 4 表示缩进级别
		jsonFile.close();
	}
	catch (const std::exception& e)
	{
		std::cerr << "An error occurred while saving the JSON configuration file: " << e.what() << std::endl;
	}

}

void config_system::open_directory()
{
	ShellExecuteA(NULL, "open", "explorer.exe", this->get_config_path().string().c_str(), NULL, SW_SHOWNORMAL);
}

std::filesystem::path config_system::get_config_path()
{
	std::filesystem::path appdata = std::getenv("APPDATA");
	std::filesystem::path path = appdata / "raot";

	if (!std::filesystem::exists(path))
	{
		std::filesystem::create_directory(path);
	}
	return path;
}

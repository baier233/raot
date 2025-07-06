#include "server.h"
#include <json.hpp>
#include "index.h"
#include <codecvt>
#include <base/features/modules/module_manager.h>
#include <base/raot.h>

static std::string url_encode(const std::string& str);
static std::string url_decode(const std::string& str);
static std::map<std::string, category > categoryMap = {
			{"1", category::COMBAT},
			{"combat", category::COMBAT},
			{"2", category::MOVEMENT},
			{"movement", category::MOVEMENT},
			{"3", category::PLAYER},
			{"player", category::PLAYER},
			{"4", category::VISUAL},
			{"visual", category::VISUAL},
			{"5", category::WORLD},
			{"world", category::WORLD},
};
#include <fstream>
#include <base/features/config/config_system.h>
void webui::server::start(int port)
{
	this->port = port;
	this->srv.Get("/", [](const httplib::Request& req, httplib::Response& res) {
		/*std::ifstream file(R"(E:\Dev\raot_client\raot-client\src\base\webui\index.html)", std::ios::binary);

		if (file) {
			file.seekg(0, std::ios::end);
			std::streamsize size = file.tellg();
			file.seekg(0, std::ios::beg);

			std::vector<char> buffer(size);
			if (file.read(buffer.data(), size)) {
				std::string content(buffer.data(), size);
				res.set_content(content, "text/html;charset=utf-8");
			}
			else {
				res.status = 500;
				res.set_content("Error reading file", "text/plain");
			}
			file.close();
		}
		else {
			res.status = 404;
			res.set_content("File not found", "text/plain");
		}
		return;*/
		static std::string index_data((char const*)index, static_cast<size_t>(index_size));
		res.set_content(index_data, "text/html;charset=utf-8");
		});

	/*
	* categoriesList
	*/
	this->srv.Get("/api/openFolder", [](const httplib::Request&, httplib::Response& res) {

		config_system::get().open_directory();
		nlohmann::json result;
		result["success"] = true;
		res.set_content(result.dump(), "application/json;charset=utf-8");
		});

	this->srv.Get("/api/loadConfig", [](const httplib::Request&, httplib::Response& res) {

		config_system::get().load_config("config.json");
		nlohmann::json result;
		result["success"] = true;
		res.set_content(result.dump(), "application/json;charset=utf-8");
		});

	this->srv.Get("/api/saveConfig", [](const httplib::Request&, httplib::Response& res) {
		config_system::get().save_config("config.json");
		nlohmann::json result;
		result["success"] = true;
		res.set_content(result.dump(), "application/json;charset=utf-8");
		});
	this->srv.Get("/api/categoriesList", [](const httplib::Request&, httplib::Response& res) {
		nlohmann::json jsonObject;
		nlohmann::json result;
		result["combat"] = "Swords";
		result["movement"] = "directions_run";
		result["player"] = "Person";
		result["visual"] = "Visibility";
		result["world"] = "Language";
		jsonObject["result"] = result;
		jsonObject["success"] = true;
		res.set_content(jsonObject.dump(), "application/json;charset=utf-8");
		});
	/*
	* setStatus
	*/
	this->srv.Get("/api/setStatus", [](const httplib::Request& req, httplib::Response& res) {
		nlohmann::json jsonObject;
		nlohmann::json result;
		std::string module_name = req.get_param_value("module");
		std::string state = req.get_param_value("state");

		auto m = raot::get().mm->get_module(module_name.c_str());
		bool states = false;

		if (state.find("true") != -1) states = true;
		else if (state.find("false") != -1) states = false;
		m->set_toggle_async(states);
		jsonObject["result"] = states;
		jsonObject["success"] = true;
		res.set_content(jsonObject.dump(), "application/json;charset=utf-8");
		});

	/*
	* setBind
	*/
	this->srv.Get("/api/setBind", [](const httplib::Request& req, httplib::Response& res) {
		nlohmann::json jsonObject;
		nlohmann::json result;
		std::string module = req.get_param_value("module");
		std::string keycode = req.get_param_value("keycode");
		std::cout << keycode << std::endl;
		int key = std::stoi(keycode);
		raot::get().mm->get_module(module.c_str())->set_key(key);
		jsonObject["result"] = key;
		jsonObject["success"] = true;
		res.set_content(jsonObject.dump(), "application/json;charset=utf-8");
		});

	/*
   * updateModulesInfo
   */
	this->srv.Get("/api/updateModulesInfo", [](const httplib::Request& req, httplib::Response& res) {
		std::string category_name = req.get_param_value("category");
		nlohmann::json jsonObject;
		nlohmann::json result;
		std::vector<HMOD> currentmods;


		if (categoryMap.find(category_name) != categoryMap.end()) {
			category enumCategory = categoryMap[category_name];
			auto& mm = raot::get().mm;
			auto moduels = mm->get_modules();
			for (auto& mod : moduels) {
				if (ToBaseModule(mod)->get_category() == enumCategory) {
					currentmods.push_back(mod);
				}
			}
			auto processModule = [&result](HMOD mod) {
				result[url_encode(ToBaseModule(mod)->get_name())] = ToBaseModule(mod)->get_toggle();
				};

			std::for_each(currentmods.begin(), currentmods.end(), processModule);
		}

		jsonObject["result"] = result;
		jsonObject["success"] = true;
		res.set_content(jsonObject.dump(), "application/json;charset=utf-8");
		});

	/*
	* modulesList
	*/
	this->srv.Get("/api/modulesList", [](const httplib::Request& req, httplib::Response& res) {
		std::string category_name = req.get_param_value("category");

		nlohmann::json jsonObject;
		nlohmann::json result;
		std::vector<HMOD> currentmods;
		//你之前写的是什么几把。。
		if (categoryMap.find(category_name) != categoryMap.end()) {
			category enumCategory = categoryMap[category_name];
			auto mm = raot::get().mm;
			for (auto& module : mm->get_modules()) {
				if (ToBaseModule(module)->get_category() == enumCategory) {
					currentmods.push_back(module);
				}
			}
			for (auto& modules : currentmods) {
				nlohmann::json moduleJsonObj;
				moduleJsonObj["state"] = ToBaseModule(modules)->get_toggle();
				moduleJsonObj["desc"] = ToBaseModule(modules)->get_desc();
				moduleJsonObj["binding"] = ToBaseModule(modules)->get_key();
				moduleJsonObj["settings"] = !ToBaseModule(modules)->get_values().empty();
				moduleJsonObj["canToggle"] = true;
				result[ToBaseModule(modules)->get_name()] = moduleJsonObj;
			}
		}

		jsonObject["result"] = result;
		jsonObject["success"] = true;
		res.set_content(jsonObject.dump(), "application/json;charset=utf-8");
		});

	/*
	* getModuleSetting
	*/
	this->srv.Get("/api/getModuleSetting", [](const httplib::Request& req, httplib::Response& res) {
		std::string moduleName = req.get_param_value("module");
		nlohmann::json jsonObject;
		bool isFound = false;
		auto mm = raot::get().mm;
		std::vector<HMOD> mods = mm->get_modules();

		for (auto& modules : mods) {
			if (ToBaseModule(modules)->get_name() == moduleName) {
				nlohmann::json moduleJsonArray;
				isFound = true;
				auto values = ToBaseModule(modules)->get_values();

				for (const auto& value : values) {
					nlohmann::json moduleSet;

					moduleSet["name"] = value.second->get_name();

					switch (value.first) {
					case values::BOOL: {
						values::bool_value* pValue = static_cast<values::bool_value*>(value.second.get());
						moduleSet["type"] = "checkbox";
						moduleSet["value"] = pValue->get_value();
						break;
					}
					case values::FLOAT: {
						values::float_value* pValue = static_cast<values::float_value*>(value.second.get());
						moduleSet["type"] = "slider";
						moduleSet["min"] = pValue->get_min();
						moduleSet["max"] = pValue->get_max();
						moduleSet["step"] = 0.1;
						moduleSet["value"] = pValue->get_value();
						break;
					}
					case values::LIST: {
						values::mode_value* pValue = static_cast<values::mode_value*>(value.second.get());
						moduleSet["type"] = "selection";
						nlohmann::json values;
						for (size_t i = 0; i < pValue->get_modes().size(); i++)
						{
							values.push_back(pValue->get_descs()[i]);
						}
						moduleSet["values"] = values;
						moduleSet["value"] = pValue->get_descs()[pValue->get_value()];
						break;
					}
					default:
						break;
					}

					if (!moduleSet.empty() && !moduleSet.is_null()) {
						moduleJsonArray.push_back(moduleSet);
					}
				}

				std::cout << moduleJsonArray.dump() << std::endl;
				jsonObject["result"] = moduleJsonArray;
			}
		}

		jsonObject["success"] = isFound;
		if (!isFound) jsonObject["reason"] = "Can't find module";
		res.set_content(jsonObject.dump(), "application/json;charset=utf-8");
		});
	/*
	* setModuleSettingValue
	*/
	this->srv.Get("/api/setModuleSettingValue", [](const httplib::Request& req, httplib::Response& res) {
		std::string moduleName = req.get_param_value("module");
		std::string name = req.get_param_value("name");
		std::string value = req.get_param_value("value");
		auto mod = raot::get().mm->get_module(moduleName.c_str());
		nlohmann::json jsonObject;
		auto values = mod->get_values();
		for (size_t i = 0; i < values.size(); i++) {
			if (values[i].second->get_name() == name) {
				switch (values[i].first) {
				case values::LIST: {
					values::mode_value* pValue = (values::mode_value*)values[i].second.get();
					nlohmann::json  values;
					for (size_t i = 0; i < pValue->get_modes().size(); i++)
					{
						if (std::string(pValue->get_descs()[i]) == value) {
							*pValue->get_value_ptr() = i;
						}
					}
					jsonObject["result"] = value;
					break;
				}
				case values::BOOL: {
					values::bool_value* pValue = (values::bool_value*)values[i].second.get();
					if (value.find("true") != -1)
					{
						*pValue->get_value_ptr() = true;
						jsonObject["result"] = true;
					}
					if (value.find("false") != -1)
					{
						*pValue->get_value_ptr() = false;
						jsonObject["result"] = false;;
					}

					break;
				}
				case values::FLOAT: {
					values::float_value* pValue = (values::float_value*)values[i].second.get();
					*pValue->get_value_ptr() = std::stof(value);
					jsonObject["result"] = std::stof(value);
					break;


				}
				}
			}
		}

		jsonObject["success"] = true;
		res.set_content(jsonObject.dump(), "application/json;charset=utf-8");
		});


	std::thread ths(&server::listen_thread, this);
	ths.detach();
}


void webui::server::detach()
{
	this->srv.stop();
}
void webui::server::listen_thread()
{
	//jni::set_thread_env(utils::jvm_utility::get_current_thread_env());
	this->srv.listen("0.0.0.0", this->port);
}

std::string url_encode(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	std::wstring wstr = conv.from_bytes(str);

	std::string encodedStr;
	for (const auto& ch : wstr) {
		if (isalnum(ch) || ch == '-' || ch == '_' || ch == '.' || ch == '~') {
			encodedStr += static_cast<char>(ch);
		}
		else if (ch == ' ') {
			encodedStr += '+';
		}
		else {
			encodedStr += '%';
			encodedStr += static_cast<char>((ch >> 4) + '0');
			encodedStr += static_cast<char>((ch & 0x0F) + '0');
		}
	}

	return encodedStr;
}

std::string url_decode(const std::string& str)
{
	std::string decodedStr;

	for (std::size_t i = 0; i < str.length(); ++i) {
		if (str[i] == '%') {
			if (i + 2 < str.length()) {
				char hex[3] = { str[i + 1], str[i + 2], '\0' };
				char decodedCh = static_cast<char>(std::stoi(hex, nullptr, 16));
				decodedStr += decodedCh;
				i += 2;
			}
		}
		else if (str[i] == '+') {
			decodedStr += ' ';
		}
		else {
			decodedStr += str[i];
		}
	}

	return decodedStr;

}

#pragma once
#include <string>
#include <utils/singleton.hpp>
#include <fstream> 
class config_system : public singleton<config_system>
{
public:
	void load_config(const std::string& name);
	void save_config(const std::string& name);
	void open_directory();
	std::filesystem::path get_config_path();
};


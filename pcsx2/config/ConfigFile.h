#pragma once

#include "Utilities/PathUtils.h"
#include <string>

class YamlConfigFile
{
private:
	// NOTE - yaml-cpp has a memory leak problem due to YAML's more advanced capabilities (making references to blocks of config)
	// to avoid this, we avoid storing / mutating it's internal data-structure and instead deserialize/serialize everytime.
	//
	// As a result - this config API should be avoided for massive files (think GameDB size) as it will result in excessive string creation, etc.
	// the nholmann::json library does not have this same issue from what I understand.
	//
	// If you need/want to use YAML for something (ie. the GameDB). It would be better to roll your own implementation where you can control
	// when and how the underlying YAML is constructed/destructed
	std::string data;
	YamlConfigFile(std::string data);
public:
	YamlConfigFile() {}

	std::string fileExtension = "yaml";

	bool loadFromFile(fs::path path);
	bool saveToFile(fs::path path);

	std::unique_ptr<YamlConfigFile> getSection(std::string key);
	std::string getString(std::string key, std::string fallback = "");
	bool getBool(std::string key, bool fallback = false);

	void setSection(std::string key, YamlConfigFile* section);
	void setString(std::string key, std::string str);
	void setBool(std::string key, bool val);
};
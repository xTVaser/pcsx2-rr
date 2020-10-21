// TODO - turn this into a polymorphic interface once working in order to facilitate easy
// drop in replacement of different configuration file formats
// OR a combination of configuration file formats.

#include <string>
#include "Utilities/PathUtils.h"

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

	void setSection(std::string key, YamlConfigFile* section);
	void setString(std::string key, std::string str);
};

// TODO - Saveable interface?

class FolderConfiguration
{
private:
	std::unique_ptr<YamlConfigFile> config;

public:
	struct Folder
	{
		std::string userPath;
		std::string defaultPath;
		bool useDefault;
	};

	FolderConfiguration(std::unique_ptr<YamlConfigFile>);

	YamlConfigFile* deserialize();

	Folder plugins;
	Folder settings;
	Folder documents;
	Folder bios;
	Folder snapshots;
	Folder savestates;
	Folder memoryCards;
	Folder languages;
	Folder logs;
	Folder cheats;
	Folder widescreenCheats;

	fs::path lastIsoPath;
	fs::path lastELFPath;
	fs::path lastDiscPath;
};

class MainConfiguration
{
private:
	std::unique_ptr<YamlConfigFile> config;
	std::unique_ptr<FolderConfiguration> folderConfig;
	YamlUtils yamlUtils;


public:
	MainConfiguration();

	void save();
};

// Global Configuration Object
class Configuration
{
public:
	Configuration();

	// Hide behind a getter, so the consumer cannot modify the underlying configuration
	MainConfiguration* mainConfig();

	void save();

private:
	std::unique_ptr<MainConfiguration> m_mainConfig;
};

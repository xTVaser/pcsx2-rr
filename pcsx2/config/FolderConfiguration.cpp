#include "PrecompiledHeader.h"

#include "FolderConfiguration.h"

#include <wx/stdpaths.h>
#include "Utilities/Path.h"
#include "App.h"

fs::path FolderConfiguration::getRootPath()
{
	switch (folderMode)
	{
#ifdef XDG_STD
		// Move all user data file into central configuration directory (XDG_CONFIG_DIR)
		case FolderMode::User:
			return GetUserLocalDataDir();
#else
		case FolderMode::User:
			return Path::Combine(Path::GetUserLocalDataDir().string(), pxGetAppName().ToStdString());
#endif
		default:
			return CustomDocumentsFolder;
	}
}

fs::path FolderConfiguration::defaultFolderPath(std::string folderName)
{
	// TODO - having a Combine for fs::path, string would be nice
	return Path::Combine(getRootPath().string(), folderName);
}

fs::path FolderConfiguration::defaultFolderPathInExecutableDir(std::string folderName)
{
	return Path::Combine(Path::GetExecutableDirectory().string(), folderName);
}

fs::path FolderConfiguration::defaultPluginsFolder()
{
	// Each linux distributions have his rules for path so we give them the possibility to
	// change it with compilation flags. -- Gregory
#ifndef PLUGIN_DIR_COMPILATION
	return Path::Combine(Path::GetExecutableDirectory().string(), "plugins");
#else
#define xPLUGIN_DIR_str(s) PLUGIN_DIR_str(s)
#define PLUGIN_DIR_str(s) #s
	return std::string(xPLUGIN_DIR_str(PLUGIN_DIR_COMPILATION));
#endif
}

fs::path FolderConfiguration::GetSettingsFolder()
{
	if (!wxGetApp().Overrides.SettingsFolder.empty())
		return wxGetApp().Overrides.SettingsFolder;

	// TODO - this bool should be passed in and be a member variable or something instead...i think
	// it comes from a yaml file, no need to be this transparent external
	return UseDefaultSettingsFolder ? PathDefs::GetSettings().string() : SettingsFolder;
}

fs::path FolderConfiguration::GetVmSettingsFilename()
{
	fs::path fname(!wxGetApp().Overrides.VmSettingsFile.empty() ? wxGetApp().Overrides.VmSettingsFile : FilenameDefs::GetVmConfig());
	std::cout << "Path: " << Path::Combine(GetSettingsFolder(), fname) << std::endl;
	return Path::Combine(GetSettingsFolder(), fname);
}

fs::path FolderConfiguration::GetUiKeysFilename()
{
	fs::path fname( FilenameDefs::GetUiKeysConfig() );
	return (GetSettingsFolder() / fname).make_preferred();
}

std::string FolderConfiguration::FullpathToBios()
{
	return Path::Combine( bios.getPath(), "BIOS TODO" ); 
}

// TODO - maybe this would be better defined in the memorycard section
std::string FolderConfiguration::FullpathToMcd(uint slot)
{
	return Path::Combine( memoryCards.getPath(), "TODO-MEMCARDS" ); // Mcd[slot].Filename.GetFullName().ToStdString()
}

FolderConfiguration::Folder::Folder(std::unique_ptr<YamlConfigFile> folderSection, std::string defaultPath)
{
	YamlConfigFile* cfg = folderSection.get();
	this->userPath = cfg->getString("userPath");
	this->defaultPath = cfg->getString("defaultPath", defaultPath);
	this->useDefault = cfg->getBool("useDefault", true);
}

fs::path FolderConfiguration::Folder::getPath()
{
	return useDefault ? defaultPath : userPath;
}

FolderConfiguration::FolderConfiguration(std::unique_ptr<YamlConfigFile> _config)
{
	// Serialize into C++ struct
	config = std::move(_config);
	plugins = Folder(config.get()->getSection("plugins"), defaultPluginsFolder());
	settings = Folder(config.get()->getSection("settings"), defaultFolderPath("settings"));
	documents = Folder(config.get()->getSection("documents"), defaultFolderPathInExecutableDir("docs"));
	bios = Folder(config.get()->getSection("bios"), defaultFolderPath("bios"));
	snapshots = Folder(config.get()->getSection("snapshots"), defaultFolderPath("snapshots"));
	savestates = Folder(config.get()->getSection("savestates"), defaultFolderPath("savestates"));
	memoryCards = Folder(config.get()->getSection("memoryCards"), defaultFolderPath("memcards"));
	languages = Folder(config.get()->getSection("languages"), defaultFolderPathInExecutableDir("langs"));
	logs = Folder(config.get()->getSection("logs"), defaultFolderPath("logs"));
	cheats = Folder(config.get()->getSection("cheats"), defaultFolderPath("cheats"));
	widescreenCheats = Folder(config.get()->getSection("widescreenCheats"), defaultFolderPath("cheats_widescreen"));

	lastIsoPath = config.get()->getString("lastIsoPath");
	lastELFPath = config.get()->getString("lastELFPath");
	lastDiscPath = config.get()->getString("lastDiscPath");
}

YamlConfigFile* FolderConfiguration::deserialize()
{
	//// shared_ptr?
	//YamlConfigFile* cfg = config.get();
	//config.get()->setString("test", bios.defaultPath); // TODO quick and dirty
	//return cfg;
	// TODO!
	return &YamlConfigFile();
}

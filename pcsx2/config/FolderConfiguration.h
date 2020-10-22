#pragma once

#include "Configuration.h"
#include "ConfigFile.h"

#include "Utilities/PathUtils.h"

// TODO - FolderFileConfiguration?

class FolderConfiguration
{
private:
	std::unique_ptr<YamlConfigFile> config;

public:
	// TODO - clean up this comment
	enum class FolderMode
	{
		// uses /home/user or /cwd for the program data.  This is the default mode and is the most
		// friendly to modern computing security requirements; as it isolates all file modification
		// to a zone of the hard drive that has granted write permissions to the user.
		User,

		// uses a custom location for program data. Typically the custom folder is either the
		// absolute or relative location of the program -- absolute is preferred because it is
		// considered more secure by MSW standards, due to DLL search rules.
		//
		// To enable PCSX2's "portable" mode, use this setting and specify "." for the custom
		// documents folder.
		Custom,
	};

	class Folder
	{
	private:
		fs::path userPath;
		fs::path defaultPath;
		bool useDefault;

	public:
		Folder() {}
		Folder(std::unique_ptr<YamlConfigFile> folderSection, std::string defaultPath);

		fs::path getPath();
	};

	FolderConfiguration(std::unique_ptr<YamlConfigFile>);
	YamlConfigFile* deserialize();

	FolderMode folderMode;

	// TODO - sensible access patterns
	fs::path getRootPath();
	fs::path defaultFolderPath(std::string folderName);
	fs::path defaultFolderPathInExecutableDir(std::string folderName);
	fs::path defaultPluginsFolder();

	// Folders that may or maynot be used else-where in the app, TBD
	fs::path GetSettingsFolder();
	fs::path GetVmSettingsFilename();
	fs::path GetUiKeysFilename();
	// TODO - store bios filename in here too instead of an external
	std::string FullpathToBios();
	// TODO - same for the memcards
	std::string FullpathToMcd(uint slot);

	// Configuration from YAML
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

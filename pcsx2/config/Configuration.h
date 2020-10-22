// TODO - turn this into a polymorphic interface once working in order to facilitate easy
// drop in replacement of different configuration file formats
// OR a combination of configuration file formats.

#include <string>
#include "Utilities/PathUtils.h"

#include "ConfigFile.h"
#include "FolderConfiguration.h"

// TODO - deserialize/save interface

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
private:
	enum class InstallationMode
	{
		// Use the user defined folder selections.  These can be anywhere on a user's hard drive,
		// though by default the binaries (plugins) are located in Install_Dir (registered
		// by the installer), and the user files (screenshots, inis) are in the user's documents
		// folder.  All folders are changable within the GUI.
		Registered,

		// In this mode, both Install_Dir and UserDocuments folders default the directory containing
		// PCSX2.exe, or the current working directory (if the PCSX2 directory could not be determined).
		// Folders cannot be changed from within the gui, however the fixed defaults can be manually
		// specified in the portable.ini by power users/devs.
		//
		// This mode is typically enabled by the presence of a 'portable.ini' in the folder.
		Portable,
	};
	InstallationMode installationMode = InstallationMode::Portable; // defaulting to portable makes....sense to me?

public:
	Configuration();

	// Hide behind a getter, so the consumer cannot modify the underlying configuration
	MainConfiguration* mainConfig();

	bool isPortableInstall();
	bool isRegisteredInstall();
	void setPortableInstall();
	void setRegisteredInstall();

	void save();

private:
	std::unique_ptr<MainConfiguration> m_mainConfig;
};

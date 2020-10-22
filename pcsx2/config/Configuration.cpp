#include "PrecompiledHeader.h"
#include "Configuration.h"

Configuration::Configuration()
	: m_mainConfig(std::make_unique<MainConfiguration>())
{
}

MainConfiguration* Configuration::mainConfig()
{
	return m_mainConfig.get();
}

bool Configuration::isPortableInstall()
{
	return installationMode == InstallationMode::Portable;
}

bool Configuration::isRegisteredInstall()
{
	return installationMode == InstallationMode::Registered;
}

void Configuration::setPortableInstall()
{
	installationMode = InstallationMode::Portable;
}

void Configuration::setRegisteredInstall()
{
	installationMode = InstallationMode::Registered;
}

void Configuration::save()
{
	m_mainConfig.get()->save();
}

MainConfiguration::MainConfiguration()
{
	// Load Main Configuration
	config = std::make_unique<YamlConfigFile>();
	// TODO - probably grab the app name instead of hardcoding pcsx2
	yamlUtils.Load(Path::Combine(Path::GetExecutableDirectory(), fs::path("pcsx2-vm").replace_extension(config.get()->fileExtension)));

	// Load Folder Configuration (and soon others)
	folderConfig = std::make_unique<FolderConfiguration>(config.get()->getSection("folders"));
}

void MainConfiguration::save()
{
	// TODO - store filename as member var (maybe on the YamlConfigFile as well...should map to a file
	
	// TODO - problem with yamlUtils interface - it requires exposing the internal data-structure of the config object
	// the load/save should be members of the config file interface, not a utility function, it can be made polymorphic/general via the interface's contract
	// yamlUtils.Save(Path::Combine(Path::GetExecutableDirectory(), fs::path("mainConfig").replace_extension(config.get()->fileExtension)), );

	// for now, I'll re-roll it here
	
	// First we have to deserialize our relevant structs
	YamlConfigFile* cfg = config.get();
	cfg->setSection("folders", folderConfig.get()->deserialize());

	// Save the file
	// TODO - probably grab the app name instead of hardcoding pcsx2, also, this should be a member variable
	config.get()->saveToFile(Path::Combine(Path::GetExecutableDirectory(), fs::path("pcsx2-vm").replace_extension(config.get()->fileExtension)));
}

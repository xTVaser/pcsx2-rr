#include "PrecompiledHeader.h"
#include "Configuration.h"

// TODO - some of this can be integrated with what is in/added to PathUtils
YamlConfigFile::YamlConfigFile(std::string data)
{
	this->data = data;
}

// TODO: instead of returning an entirely new node, it should contain a reference to the original 
// but have a growing list of keys to access a nested element transparently.
// that way, everything modifying the same underlying source of truth / reduces duplication / keeps the interface simplified
std::unique_ptr<YamlConfigFile> YamlConfigFile::getSection(std::string key)
{
	YAML::Node node = YAML::Load(data);
	std::ostringstream os;
	os << node[key];
	return std::make_unique<YamlConfigFile>(YamlConfigFile(os.str()));
}

std::string YamlConfigFile::getString(std::string key, std::string fallback)
{
	// TODO - parse an invalid string!
	YAML::Node node = YAML::Load(data);
	return !node[key] ? fallback : node[key].as<std::string>();
}

Configuration::Configuration()
	: m_mainConfig(std::make_unique<MainConfiguration>())
{
}

MainConfiguration* Configuration::mainConfig()
{
	return m_mainConfig.get();
}

void Configuration::save()
{
	m_mainConfig.get()->save();
}

MainConfiguration::MainConfiguration()
{
	// Load Main Configuration
	config = std::make_unique<YamlConfigFile>();
	yamlUtils.Load(Path::Combine(Path::GetExecutableDirectory(), fs::path("mainConfig").replace_extension(config.get()->fileExtension)));

	// Load Folder Configuration (and soon others)
	folderConfig = std::make_unique<FolderConfiguration>(config.get());
}

void MainConfiguration::save()
{
	// TODO - store filename as member var (maybe on the YamlConfigFile as well...should map to a file
	//yamlUtils.Save(Path::Combine(Path::GetExecutableDirectory(), fs::path("mainConfig").replace_extension(config.get()->fileExtension)));
}

FolderConfiguration::FolderConfiguration(YamlConfigFile* parentConfig)
{
	// Serialize into C++ struct
	config = parentConfig->getSection("folders");
	bios = Folder({config->getString("test", "testPath"), "test", false});
}

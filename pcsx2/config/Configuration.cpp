#include "PrecompiledHeader.h"
#include "Configuration.h"

// TODO - some of this can be integrated with what is in/added to PathUtils
YamlConfigFile::YamlConfigFile(std::string data)
{
	this->data = data;
}

bool YamlConfigFile::saveToFile(fs::path path)
{
	YAML::Node node = YAML::Load(data);
	std::ofstream fout(path);
	fout << node;
	return true;
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

void YamlConfigFile::setSection(std::string key, YamlConfigFile* section)
{
	std::string sectionData = section->data; // TODO - is this really possible on a private variable?
	YAML::Node sectionNode = YAML::Load(sectionData);

	YAML::Node currentNode = YAML::Load(data);
	currentNode[key] = sectionNode;

	// re-init yaml, this is duplicate code
	std::ostringstream os;
	os << currentNode;
	data = os.str();
}

void YamlConfigFile::setString(std::string key, std::string str)
{
	YAML::Node node = YAML::Load(data);
	node[key] = str;

	// re-init yaml, this is duplicate code
	std::ostringstream os;
	os << node;
	data = os.str();
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
	config.get()->saveToFile(Path::Combine(Path::GetExecutableDirectory(), fs::path("mainConfig").replace_extension(config.get()->fileExtension)));
}

FolderConfiguration::FolderConfiguration(std::unique_ptr<YamlConfigFile> _config)
{
	// Serialize into C++ struct
	config = std::move(_config);
	bios = Folder({config->getString("test", "testPath"), "test", false});
}

YamlConfigFile* FolderConfiguration::deserialize()
{
	// shared_ptr?
	YamlConfigFile* cfg = config.get();
	config.get()->setString("test", bios.defaultPath); // TODO quick and dirty
	return cfg;
}

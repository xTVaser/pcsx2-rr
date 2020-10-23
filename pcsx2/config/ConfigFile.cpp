#include "PrecompiledHeader.h"

#include "ConfigFile.h"

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
std::shared_ptr<YamlConfigFile> YamlConfigFile::getSection(std::string key)
{
	YAML::Node node = YAML::Load(data);
	std::ostringstream os;
	os << node[key];
	return std::make_shared<YamlConfigFile>(YamlConfigFile(os.str()));
}

std::string YamlConfigFile::getString(std::string key, std::string fallback)
{
	// TODO - parse an invalid string!
	YAML::Node node = YAML::Load(data);
	return !node[key] ? fallback : node[key].as<std::string>();
}

char YamlConfigFile::getChar(std::string key, char fallback)
{
	return 0;
}

void YamlConfigFile::setSection(std::string key, std::shared_ptr<YamlConfigFile> section)
{
	std::string sectionData = section.get()->data; // TODO - is this really possible on a private variable?
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

void YamlConfigFile::setChar(std::string key, char val)
{
}

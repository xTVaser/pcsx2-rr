#include "PrecompiledHeader.h"

#include "ConfigFile.h"

// TODO list
// error handling / parsing should involve a try-catch,  saving is likely safe

// TODO - some of this can be integrated with what is in/added to PathUtils
YamlConfigFile::YamlConfigFile(std::string data)
{
	this->data = data;
}

bool YamlConfigFile::loadFromFile(fs::path path)
{
	// TODO - handle if the file can't be loaded
	YAML::Node node = YAML::LoadFile(path);

	std::ostringstream os;
	os << node;
	data = os.str();

	return false;
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

// TODO - template function or something to hold implementaiton?
bool YamlConfigFile::getBool(std::string key, bool fallback)
{
	YAML::Node node = YAML::Load(data);
	return !node[key] ? fallback : node[key].as<bool>();
}

char YamlConfigFile::getChar(std::string key, char fallback)
{
	YAML::Node node = YAML::Load(data);
	return !node[key] ? fallback : node[key].as<char>();
}

int YamlConfigFile::getInt(std::string key, int fallback)
{
	YAML::Node node = YAML::Load(data);
	return !node[key] ? fallback : node[key].as<int>();
}

uint32_t YamlConfigFile::getU32(std::string key, uint32_t fallback)
{
	YAML::Node node = YAML::Load(data);
	return !node[key] ? fallback : node[key].as<uint32_t>();
}

float YamlConfigFile::getFloat(std::string key, float fallback)
{
	YAML::Node node = YAML::Load(data);
	return !node[key] ? fallback : node[key].as<float>();
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

// TODO - this is all duplicate, template function maybe?
void YamlConfigFile::setString(std::string key, std::string str)
{
	YAML::Node node = YAML::Load(data);
	node[key] = str;

	// re-init yaml, this is duplicate code
	std::ostringstream os;
	os << node;
	data = os.str();
}

void YamlConfigFile::setBool(std::string key, bool val)
{
	YAML::Node node = YAML::Load(data);
	node[key] = val;

	// re-init yaml, this is duplicate code
	std::ostringstream os;
	os << node;
	data = os.str();
}

void YamlConfigFile::setChar(std::string key, char val)
{
	YAML::Node node = YAML::Load(data);
	node[key] = val;

	// re-init yaml, this is duplicate code
	std::ostringstream os;
	os << node;
	data = os.str();
}

void YamlConfigFile::setInt(std::string key, int val)
{
	YAML::Node node = YAML::Load(data);
	node[key] = val;

	// re-init yaml, this is duplicate code
	std::ostringstream os;
	os << node;
	data = os.str();
}

void YamlConfigFile::setU32(std::string key, uint32_t val)
{
	YAML::Node node = YAML::Load(data);
	node[key] = val;

	// re-init yaml, this is duplicate code
	std::ostringstream os;
	os << node;
	data = os.str();
}

void YamlConfigFile::setFloat(std::string key, float val)
{
	YAML::Node node = YAML::Load(data);
	node[key] = val;

	// re-init yaml, this is duplicate code
	std::ostringstream os;
	os << node;
	data = os.str();
}

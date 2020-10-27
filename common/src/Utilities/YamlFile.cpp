#include "PrecompiledHeader.h"

#include "yaml-cpp/yaml.h"
#include "YamlFile.h"

// TODO list
// error handling / parsing should involve a try-catch,  saving is likely safe

// TODO - some of this can be integrated with what is in/added to PathUtils
YamlFile::YamlFile(std::string data)
{
	this->data = data;
}

bool YamlFile::loadFromFile(fs::path path)
{
	try
	{
		YAML::Node node = YAML::LoadFile(path);
		std::ostringstream os;
		os << node;
		data = os.str();
		return true;
	}
	catch (const std::exception& e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl;
		data = "";
		return false;
	}
}

bool YamlFile::saveToFile(fs::path path)
{
	YAML::Node node;
	try
	{
	    node = YAML::Load(data);
	}
	catch (const std::exception& e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl;
	}

	std::ofstream fout(path);
	fout << node;
	return true;
}

// TODO: instead of returning an entirely new node, it should contain a reference to the original
// but have a growing list of keys to access a nested element transparently.
// that way, everything modifying the same underlying source of truth / reduces duplication / keeps the interface simplified
std::shared_ptr<YamlFile> YamlFile::getSection(std::string key)
{
	YAML::Node node = YAML::Load(data);
	std::ostringstream os;
	os << node[key];
	return std::make_shared<YamlFile>(YamlFile(os.str()));
}

std::string YamlFile::getString(std::string key, std::string fallback)
{
	// TODO - parse an invalid string!
	YAML::Node node = YAML::Load(data);
	return !node[key] ? fallback : node[key].as<std::string>();
}

// TODO - template function or something to hold implementaiton?
bool YamlFile::getBool(std::string key, bool fallback)
{
	YAML::Node node = YAML::Load(data);
	return !node[key] ? fallback : node[key].as<bool>();
}

char YamlFile::getChar(std::string key, char fallback)
{
	YAML::Node node = YAML::Load(data);
	return !node[key] ? fallback : node[key].as<char>();
}

int YamlFile::getInt(std::string key, int fallback)
{
	YAML::Node node = YAML::Load(data);
	return !node[key] ? fallback : node[key].as<int>();
}

uint32_t YamlFile::getU32(std::string key, uint32_t fallback)
{
	YAML::Node node = YAML::Load(data);
	return !node[key] ? fallback : node[key].as<uint32_t>();
}

float YamlFile::getFloat(std::string key, float fallback)
{
	YAML::Node node = YAML::Load(data);
	return !node[key] ? fallback : node[key].as<float>();
}

void YamlFile::setSection(std::string key, std::shared_ptr<YamlFile> section)
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
void YamlFile::setString(std::string key, std::string str)
{
	YAML::Node node = YAML::Load(data);
	node[key] = str;

	// re-init yaml, this is duplicate code
	std::ostringstream os;
	os << node;
	data = os.str();
}

void YamlFile::setBool(std::string key, bool val)
{
	YAML::Node node = YAML::Load(data);
	node[key] = val;

	// re-init yaml, this is duplicate code
	std::ostringstream os;
	os << node;
	data = os.str();
}

void YamlFile::setChar(std::string key, char val)
{
	YAML::Node node = YAML::Load(data);
	node[key] = val;

	// re-init yaml, this is duplicate code
	std::ostringstream os;
	os << node;
	data = os.str();
}

void YamlFile::setInt(std::string key, int val)
{
	YAML::Node node = YAML::Load(data);
	node[key] = val;

	// re-init yaml, this is duplicate code
	std::ostringstream os;
	os << node;
	data = os.str();
}

void YamlFile::setU32(std::string key, uint32_t val)
{
	YAML::Node node = YAML::Load(data);
	node[key] = val;

	// re-init yaml, this is duplicate code
	std::ostringstream os;
	os << node;
	data = os.str();
}

void YamlFile::setFloat(std::string key, float val)
{
	YAML::Node node = YAML::Load(data);
	node[key] = val;

	// re-init yaml, this is duplicate code
	std::ostringstream os;
	os << node;
	data = os.str();
}



// TODO - take anything from here that's needed

//bool JsonUtils::Save(fs::path toSave, std::string stream) // A Møøse once bit my sister... 
//{
//    if (folder.DoesExist(toSave))
//    {
//        fs::resize_file(toSave, 0);
//    }
//
//    out.open(toSave, std::ios::out | std::ios::trunc);
//    out.seekp(0);
//    out << std::setw(4) << stream; 
//    out.close();
//
//	if (out.bad())
//	{
//		std::cout << "Error Writing File" << std::endl;
//        return false;
//    }
//       
//    return true;
//}
//
//bool JsonUtils::Load(fs::path toLoad) // We apologise for the fault in the comments. Those responsible have been sacked.
//{
//
//    in.open(toLoad);
//    
//    try
//    {
//        stream = nlohmann::json::parse(in);
//    }
//    catch(const nlohmann::json::exception& e)
//    {
//        std::cerr << e.what() << '\n';
//        return false;
//    }
//    
//    in.close();
//
//    if (in.fail())
//    {
//        return false;
//    }
//    else
//    {
//    return true;
//    }
//}
//
//nlohmann::json JsonUtils::GetStream()
//{
//    return stream;
//}
//
//
//bool YamlUtils::Save(fs::path toSave, std::string stream)
//{
//    if (folder.DoesExist(toSave))
//    {
//        fs::resize_file(toSave, 0);
//    }
//
//    out.open(toSave, std::ios::out | std::ios::trunc);
//    out.seekp(0);
//    out << stream; 
//    out.close();
//
//	if (out.bad())
//	{
//		std::cout << "Error Writing File" << std::endl;
//        return false;
//    }
//       
//    return true;
//}
//
//bool YamlUtils::Load(fs::path toLoad)
//{
//    try
//    {
//        stream = YAML::LoadFile(toLoad);
//        std::ostringstream os;
//		os << stream;
//		data = os.str();
//    }
//	catch (const std::exception& e)
//	{
//		return false;
//	}
//
//    return true;
//}
//
//    YAML::Node YamlUtils::GetStream()
//    {
//        return stream;
//    }
#include "PrecompiledHeader.h"

#include "fmt/core.h"
#include "PortableConfig.h"


PortableConfig::PortableConfig()
{
	file = std::make_unique<YamlFile>();
	m_filePath = getPortableFilePath();
}

fs::path PortableConfig::getPortableFilePath()
{
	fs::path programDir = Path::GetExecutableDirectory();
	return Path::Combine(programDir.string(), fmt::format("portable.%s", file.get()->fileExtension()));
}

bool PortableConfig::load()
{
	// TODO - generic error handling in `loadFromFile` would be nice
	try
	{
		YamlFile* cfg = file.get();
		cfg->loadFromFile(m_filePath);
		m_runWizard = cfg->getBool("RunWizard", false);
		return true;
	}
	catch (std::exception ex)
	{
		// TODO - log (config source would maybe be nice?)
		return false;
	}
}

bool PortableConfig::save()
{
	YamlFile* cfg = file.get();
	cfg->setBool("RunWizard", true);
	cfg->saveToFile(m_filePath);
}

void PortableConfig::setRunWizard(bool runWizard)
{
	m_runWizard = runWizard;
}

bool PortableConfig::getRunWizard()
{
	return m_runWizard;
}

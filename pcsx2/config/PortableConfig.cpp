#include "PrecompiledHeader.h"

#include "fmt/core.h"
#include "PortableConfig.h"


PortableConfig::PortableConfig()
{
	file = std::make_unique<YamlFile>();
	m_filePath = getFilePath();
}

fs::path PortableConfig::getFilePath()
{
	fs::path programDir = Path::GetExecutableDirectory();
	return Path::Combine(programDir.string(), fmt::format("portable.{}", file.get()->fileExtension()));
}

bool PortableConfig::load()
{
	// TODO - generic error handling in `loadFromFile` would be nice
	try
	{
		file->loadFromFile(m_filePath);
		m_runWizard = file->getBool("RunWizard", false);
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
	file->setBool("RunWizard", true);
	file->saveToFile(m_filePath);
	return true;
}

void PortableConfig::setRunWizard(bool runWizard)
{
	m_runWizard = runWizard;
}

bool PortableConfig::getRunWizard()
{
	return m_runWizard;
}

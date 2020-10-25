#pragma once

#include "Utilities/PathUtils.h"

class PortableConfig
{
private:
	std::unique_ptr<YamlFile> file;

	bool m_runWizard;
	fs::path m_filePath;

public:
	PortableConfig();

	fs::path getFilePath();

	bool load();
	bool save();

	void setRunWizard(bool runWizard);
	bool getRunWizard();
};

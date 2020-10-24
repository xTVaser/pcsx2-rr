#pragma once

#include "Utilities/PathUtils.h"
#include <string>

// TODO - switch to runtime/compile time polymorphism (NVI/CRTP pattern)
// (everything can be a generic <ConfigFile> object, via the common interface we can
// easily refactor to another type / use json/yaml/intermixed, etc.
class iFile
{
	// TODO - add to interface as needed
public:
	virtual std::string fileExtension() = 0;
	virtual bool loadFromFile(fs::path path) = 0;
	virtual bool saveToFile(fs::path path) = 0;
};

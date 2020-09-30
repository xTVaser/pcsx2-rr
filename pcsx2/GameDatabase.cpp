/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2020  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"

#include "GameDatabase.h"

#include "yaml-cpp/yaml.h"
#include <iostream>

bool YamlGameDatabaseImpl::initDatabase(const std::string filePath)
{
	std::ifstream dbFile(filePath);

	if (!dbFile.good())
	{
		std::cerr << "Can't open the given gamedb file\n";
		return false;
	}

	// TODO - wrap for parser errors
	// invalid yaml seems to throw an abort, not great, but it might have just been because i gave it a REALLY screwed up file
	try
	{
		// yaml-cpp definitely takes longer to parse this giant file, but the library feels more mature
		// rapidyaml exists and I have it mostly setup which could be easily dropped in as a replacement
		// the problem i ran into with rapidyaml is there is a lack of usage/documentation as it's new
		// and i didn't like the default way it handles exceptions.
		//
		// TODO - make sure this happens in a separate thread
		// TODO - exception handling - invalid yaml?
		gameDb = YAML::LoadFile(filePath);
	}
	catch (const std::exception& e)
	{
		std::cout << "TODO!";
	}
}

/// TODO - the following helper functions can realistically be put in some sort of general yaml utility library
// TODO - might be a way to condense this with templates? get it working first
std::string safeGetString(const YAML::Node& n, std::string key, std::string default = "")
{
	if (!n[key])
		return default;
	// TODO - test this safety consideration (parse a value that isn't actually a string
	return n[key].as<std::string>();
}

int safeGetInt(const YAML::Node& n, std::string key, int default = 0)
{
	if (!n[key])
		return default;
	// TODO - test this safety consideration (parse a value that isn't actually a string
	return n[key].as<int>();
}

std::vector<std::string> safeGetStringList(const YAML::Node& n, std::string key, std::vector<std::string> default = {})
{
	if (!n[key])
		return default;
	// TODO - test this safety consideration (parse a value that isn't actually a string
	return n[key].as<std::vector<std::string>>();
}

void operator>>(const YAML::Node& n, GameDatabaseSchema::GameEntry& v)
{
	try
	{
		v.name = safeGetString(n, "name");
		v.region = safeGetString(n, "region");
		v.compat = static_cast<GameDatabaseSchema::Compatibility>(safeGetInt(n, "compat"));
		v.eeRoundMode = static_cast<GameDatabaseSchema::RoundMode>(safeGetInt(n, "eeRoundMode"));
		v.vuRoundMode = static_cast<GameDatabaseSchema::RoundMode>(safeGetInt(n, "vuRoundMode"));
		v.eeClampMode = static_cast<GameDatabaseSchema::ClampMode>(safeGetInt(n, "eeClampMode"));
		v.vuClampMode = static_cast<GameDatabaseSchema::ClampMode>(safeGetInt(n, "vuClampMode"));
		v.gameFixes = safeGetStringList(n, "gameFixes");
		v.speedHacks = safeGetStringList(n, "speedHacks");
		v.memcardFilters = safeGetStringList(n, "memcardFilters");

		if (YAML::Node patches = n["patches"])
		{
			for (YAML::const_iterator it = patches.begin(); it != patches.end(); ++it)
			{
				YAML::Node key = it->first;
				YAML::Node val = it->second;
				GameDatabaseSchema::Patch patch;
				patch.crc = safeGetString(val, "crc");
				patch.content = safeGetStringList(val, "content");
				v.patches.push_back(patch);
			}
		}
	}
	catch (std::exception& e)
	{
		v.isValid = false;
	}
}

// TODO - yaml error handling - https://github.com/biojppm/rapidyaml#custom-allocators-and-error-handlers

GameDatabaseSchema::GameEntry YamlGameDatabaseImpl::findGame(const std::string serial)
{
	GameDatabaseSchema::GameEntry entry;
	if (YAML::Node game = gameDb[serial])
	{
		game >> entry;
		return entry;
	}
	entry.isValid = false;
	return entry;
}

// TODO - why is there a create game option, is writing to the file from the application...actually a thing...it has to be commited to the repo?
// (i think just badly named, investigate)
bool YamlGameDatabaseImpl::createNewGame(const std::string serial, GameDatabaseSchema::GameEntry& entry)
{
	return true;
}

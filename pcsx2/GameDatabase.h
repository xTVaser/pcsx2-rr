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

#pragma once

#include "yaml-cpp/yaml.h"

#include <vector>
#include <string>

// Since this is kinda yaml specific, might be a good idea to
// relocate this into the yaml class
// or put the serialization methods inside the yaml
class GameDatabaseSchema
{
public:
	enum class Compatibility
	{
		Unknown = 0,
		Nothing,
		Intro,
		Menu,
		InGame,
		Playable,
		Perfect
	};

	enum class RoundMode
	{
		Nearest = 0,
		NegativeInfinity,
		PositiveInfinity,
		ChopZero
	};

	enum class ClampMode
	{
		Disabled = 0,
		Normal,
		Extra,
		Full
	};

	// No point in using enums because i need to convert from a string then
	// left here incase i turn these into lists to validate against
	/*enum class GameFix
	{
		VuAddSubHack = 0,
		FpuCompareHack,
		FpuMulHack,
		FpuNegDivHack,
		XgKickHack,
		IPUWaitHack,
		EETimingHack,
		SkipMPEGHack,
		OPHFLagHack,
		DMABusyHack,
		VIFFIFOHack,
		VIF1StallHack,
		GIFFIFOHack,
		FMVinSoftwareHack,
		ScarfaceIbitHack,
		CrashTagTeamRacingIbit,
		VU0KickstartHack,
	};

	enum class SpeedHacks
	{
		mvuFlagSpeedHack = 0
	};*/

	struct Patch
	{
		std::string crc;
		std::vector<std::string> content;
	};

	struct GameEntry
	{
		bool isValid = true;
		std::string name;
		std::string region;
		Compatibility compat = Compatibility::Unknown;
		RoundMode eeRoundMode = RoundMode::Nearest;
		RoundMode vuRoundMode = RoundMode::Nearest;
		ClampMode eeClampMode = ClampMode::Disabled;
		ClampMode vuClampMode = ClampMode::Disabled;
		std::vector<std::string> gameFixes;
		std::vector<std::string> speedHacks;
		std::vector<std::string> memcardFilters;
		std::vector<Patch> patches;
	};
};

class IGameDatabase
{
public:
	virtual bool initDatabase(const std::string filePath) = 0;
	virtual GameDatabaseSchema::GameEntry findGame(const std::string serial) = 0;
	virtual bool createNewGame(const std::string serial, GameDatabaseSchema::GameEntry& entry) = 0;
};

class YamlGameDatabaseImpl : public IGameDatabase
{
public:
	bool initDatabase(const std::string filePath) override;

	// For reading from YAML, no need to write unless we are saving to the file...which we aren't
	
	friend void operator >> (const YAML::Node& node, GameDatabaseSchema::GameEntry& v);

	int safeGetInt(const YAML::Node& n, std::string key, int def);
	std::string safeGetString(const YAML::Node& n, std::string key, std::string def);

	std::vector<std::string> safeGetStringList(const YAML::Node& n, std::string key, std::vector<std::string> def);

	// TODO - i got rid of returning pointer types because...why?
	// if the consumer wants a pointer, they can handle that but we retain a reference to the parsed db anyway
	GameDatabaseSchema::GameEntry findGame(const std::string serial) override;
	bool createNewGame(const std::string serial, GameDatabaseSchema::GameEntry& entry) override;

private:
	YAML::Node gameDb;
};

extern IGameDatabase* AppHost_GetGameDatabase();

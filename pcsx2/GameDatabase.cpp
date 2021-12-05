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

#include "common/StringUtil.h"

#include <chrono>
#include <sstream>
#include "fmt/core.h"
#include "fmt/ranges.h"
#include <fstream>

#include <wx/stdpaths.h>

YamlGameDatabaseImpl g_gameDB;

std::string GameDatabaseSchema::GameEntry::memcardFiltersAsString() const
{
	return fmt::to_string(fmt::join(memcardFilters, "/"));
}

const GameDatabaseSchema::Patch* GameDatabaseSchema::GameEntry::findPatch(const std::string_view& crc) const
{
	std::string crcLower = StringUtil::toLower(crc);
	Console.WriteLn(fmt::format("[GameDB] Searching for patch with CRC '{}'", crc));
	if (patches.count(crcLower) == 1)
	{
		Console.WriteLn(fmt::format("[GameDB] Found patch with CRC '{}'", crc));
		return &patches.at(crcLower);
	}
	else if (patches.count("default") == 1)
	{
		Console.WriteLn("[GameDB] Found and falling back to default patch");
		return &patches.at("default");
	}
	Console.WriteLn("[GameDB] No CRC-specific patch or default patch found");
	return nullptr;
}

std::string GameDatabaseSchema::GameEntry::compatAsString() const
{
	switch (compat)
	{
		case GameDatabaseSchema::Compatibility::Perfect:
			return "Perfect";
		case GameDatabaseSchema::Compatibility::Playable:
			return "Playable";
		case GameDatabaseSchema::Compatibility::InGame:
			return "In-Game";
		case GameDatabaseSchema::Compatibility::Menu:
			return "Menu";
		case GameDatabaseSchema::Compatibility::Intro:
			return "Intro";
		case GameDatabaseSchema::Compatibility::Nothing:
			return "Nothing";
		default:
			return "Unknown";
	}
}

std::vector<std::string> YamlGameDatabaseImpl::convertMultiLineStringToVector(const std::string& multiLineString)
{
	std::vector<std::string> lines;
	std::istringstream stream(multiLineString);
	std::string line;
	while (std::getline(stream, line))
	{
		lines.push_back(line);
	}
	return lines;
}

GameDatabaseSchema::GameEntry YamlGameDatabaseImpl::entryFromYaml(const std::string_view& serial, const c4::yml::NodeRef& node)
{
	GameDatabaseSchema::GameEntry gameEntry;
	try
	{
		if (node.has_child("name"))
		{
			node["name"] >> gameEntry.name;
		}
		if (node.has_child("region"))
		{
			node["region"] >> gameEntry.region;
		}
		if (node.has_child("compat"))
		{
			int val = 0;
			node["compat"] >> val;
			gameEntry.compat = static_cast<GameDatabaseSchema::Compatibility>(val);
		}
		if (node.has_child("roundModes"))
		{
			if (node.has_child("eeRoundMode"))
			{
				int eeVal = -1;
				node["eeRoundMode"] >> eeVal;
				gameEntry.eeRoundMode = static_cast<GameDatabaseSchema::RoundMode>(eeVal);
			}
			if (node.has_child("vuRoundMode"))
			{
				int vuVal = -1;
				node["vuRoundMode"] >> vuVal;
				gameEntry.vuRoundMode = static_cast<GameDatabaseSchema::RoundMode>(vuVal);
			}
		}
		if (node.has_child("clampModes"))
		{
			if (node.has_child("eeClampMode"))
			{
				int eeVal = -1;
				node["eeClampMode"] >> eeVal;
				gameEntry.eeClampMode = static_cast<GameDatabaseSchema::ClampMode>(eeVal);
			}
			if (node.has_child("vuClampMode"))
			{
				int vuVal = -1;
				node["vuClampMode"] >> vuVal;
				gameEntry.vuClampMode = static_cast<GameDatabaseSchema::ClampMode>(vuVal);
			}
		}

		// Validate game fixes, invalid ones will be dropped!
		if (node.has_child("gameFixes") && node["gameFixes"].has_children())
		{
			for (const ryml::NodeRef& n : node["gameFixes"].children())
			{
				bool fixValidated = false;
				auto fix = std::string(n.val().str, n.val().len);
				for (GamefixId id = GamefixId_FIRST; id < pxEnumEnd; id++)
				{
					std::string validFix = fmt::format("{}Hack", wxString(EnumToString(id)).ToUTF8());
					if (validFix == fix)
					{
						fixValidated = true;
						break;
					}
				}
				if (fixValidated)
				{
					gameEntry.gameFixes.emplace_back(std::move(fix));
				}
				else
				{
					Console.Error(fmt::format("[GameDB] Invalid gamefix: '{}', specified for serial: '{}'. Dropping!", fix, serial));
				}
			}
		}

		// Validate speed hacks, invalid ones will be dropped!
		if (node.has_child("speedHacks") && node["speedHacks"].has_children())
		{
			for (const ryml::NodeRef& n : node["speedHacks"].children())
			{
				bool speedHackValidated = false;
				auto speedHack = std::string(n.key().str, n.key().len);
				for (SpeedhackId id = SpeedhackId_FIRST; id < pxEnumEnd; id++)
				{
					std::string validSpeedHack = fmt::format("{}SpeedHack", wxString(EnumToString(id)).ToUTF8());
					if (validSpeedHack == speedHack)
					{
						speedHackValidated = true;
						break;
					}
				}
				if (speedHackValidated)
				{
					gameEntry.speedHacks[speedHack] = std::atoi(n.val().str);
				}
				else
				{
					Console.Error(fmt::format("[GameDB] Invalid speedhack: '{}', specified for serial: '{}'. Dropping!", speedHack, serial));
				}
			}
		}

		// Memory Card Filters - Store as a vector to allow flexibility in the future
		// - currently they are used as a '\n' delimited string in the app
		if (node.has_child("memcardFilters") && node["memcardFilters"].has_children())
		{
			for (const ryml::NodeRef& n : node["memcardFilters"].children())
			{
				auto memcardFilter = std::string(n.val().str, n.val().len);
				gameEntry.memcardFilters.emplace_back(std::move(memcardFilter));
			}
		}

		// Game Patches
		if (node.has_child("patches") && node["patches"].has_children())
		{
			for (const ryml::NodeRef& n : node["patches"].children())
			{
				auto crc = std::string(n.key().str, n.key().len);
				if (gameEntry.patches.count(crc) == 1)
				{
					Console.Error(fmt::format("[GameDB] Duplicate CRC '{}' found for serial: '{}'. Skipping, CRCs are case-insensitive!", crc, serial));
					continue;
				}
				GameDatabaseSchema::Patch patch;
				if (n.has_child("author"))
				{
					n["author"] >> patch.author;
				}
				if (n.has_child("content"))
				{
					std::string patchLines;
					n["content"] >> patchLines;
					patch.patchLines = convertMultiLineStringToVector(patchLines);
				}
				gameEntry.patches[crc] = patch;
			}
		}
	}
	// TODO - test ryml exception handling
	catch (const std::exception& e)
	{
		Console.Error(fmt::format("[GameDB] Unexpected error occurred when reading serial: '{}'. Error Details - {}", serial, e.what()));
		gameEntry.isValid = false;
	}
	return gameEntry;
}

// NOTE - when we load more files relative from the .exe, we'll want to isolate this
// to insulate the GameDB (and others) from the GUI.  At the moment though, this is the only file.
std::optional<wxString> getFileAsStream()
{
	// TODO - config - refactor with std::filesystem/ghc::filesystem
	wxString file("GameIndex.yaml");
	if (wxFileName(file).IsRelative())
	{
		// InstallFolder is the preferred base directory for the DB file, but the registry can point to previous
		// installs if uninstall wasn't done properly.
		// Since the games DB file is considered part of pcsx2.exe itself, look for it at the exe folder
		//   regardless of any other settings.

		// Note 1: Portable setup didn't suffer from this as install folder pointed already to the exe folder in portable.
		// Note 2: Other folders are either configurable (memcards, etc) or create their content automatically (inis)
		//           So the games DB was really the only one that suffers from residues of prior installs.

		//wxDirName dir = InstallFolder;
		wxDirName dir = (wxDirName)wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();
		file = (dir + file).GetFullPath();
	}

	if (!wxFileExists(file))
	{
		Console.Error(L"[GameDB] Database Not Found! [%s]", WX_STR(file));
		return std::nullopt;
	}
	return file;
}

std::ifstream getFileStream(wxString path)
{
#ifdef _WIN32
	return std::ifstream(path.wc_str());
#else
	return std::ifstream(path.c_str());
#endif
}

void YamlGameDatabaseImpl::initDatabase()
{
	const ryml::Callbacks preserve_callbacks = ryml::get_callbacks();
	const c4::error_callback_type preserve_c4_callback = c4::get_error_callback();
	auto callbacks = ryml::get_callbacks();
	callbacks.m_error = [](const char* msg, size_t msg_len, ryml::Location loc, void*) {
		throw std::runtime_error(fmt::format("[YAML] Parsing error at {}:{} (bufpos={}): {}",
			loc.line, loc.col, loc.offset, msg));
	};
	c4::set_error_callback([](const char* msg, size_t msg_size) {
		throw std::runtime_error(fmt::format("[YAML] Internal Parsing error: {}",
			msg));
	});
	try
	{
		auto maybe_FilePath = getFileAsStream();
		if (!maybe_FilePath.has_value())
		{
			Console.Error("[GameDB] Unable to open GameDB file.");
			return;
		}

		auto dbStream = getFileStream(maybe_FilePath.value());

		dbStream.seekg(0, std::ios::end);
		const size_t size = dbStream.tellg();
		dbStream.seekg(0, std::ios::beg);

		std::vector<char> buf(size);
		dbStream.read(buf.data(), size);
		dbStream.close();

		const ryml::substr view = c4::basic_substring<char>(buf.data(), size);
		ryml::Tree tree = ryml::parse(view);
		ryml::NodeRef root = tree.rootref();

		for (const ryml::NodeRef& n : root.children())
		{
			auto serial = StringUtil::toLower(std::string(n.key().str, n.key().len));

			// Serials and CRCs must be inserted as lower-case, as that is how they are retrieved
			// this is because the application may pass a lowercase CRC or serial along
			//
			// However, YAML's keys are as expected case-sensitive, so we have to explicitly do our own duplicate checking
			if (gameDb.count(serial) == 1)
			{
				Console.Error(fmt::format("[GameDB] Duplicate serial '{}' found in GameDB. Skipping, Serials are case-insensitive!", serial));
				continue;
			}
			if (n.is_map())
			{
				gameDb[serial] = entryFromYaml(serial, n);
			}
		}
	}
	catch (const std::exception& e)
	{
		Console.Error(fmt::format("[GameDB] Error occured when initializing GameDB: {}", e.what()));
		return;
	}
	ryml::set_callbacks(preserve_callbacks);
	c4::set_error_callback(preserve_c4_callback);
}

const GameDatabaseSchema::GameEntry* YamlGameDatabaseImpl::findGame(const std::string_view& serial)
{
	// Lazy load the database if it hasn't been already
	std::call_once(db_loaded, [this]() {
		const auto begin = std::chrono::steady_clock::now();
		Console.WriteLn(fmt::format("[GameDB] Has not been initialized yet, initializing..."));
		initDatabase();
		const auto end = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
		Console.WriteLn(fmt::format("[GameDB] Initialized in {}ms! {} games on record.", duration, gameDb.size()));
	});

	std::string serialLower = StringUtil::toLower(serial);
	Console.WriteLn(fmt::format("[GameDB] Searching for '{}' in GameDB", serialLower));
	const auto gameEntry = gameDb.find(serialLower);
	if (gameEntry != gameDb.end())
	{
		Console.WriteLn(fmt::format("[GameDB] Found '{}' in GameDB", serialLower));
		return &gameEntry->second;
	}

	Console.Error(fmt::format("[GameDB] Could not find '{}' in GameDB", serialLower));
	return nullptr;
}

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

#include "App.h"
#include "AppGameDatabase.h"

AppGameDatabase& AppGameDatabase::LoadFromFile(const wxString& _file, const wxString& key)
{
	// TODO - remove wx usage from here, but also get the right path, hard-coding for simplicity on my part
	wxString file("C:\\Users\\xtvas\\Repositories\\pcsx2\\bin\\GameIndex.yaml");
	if (wxFileName(file).IsRelative())
	{
		// InstallFolder is the preferred base directory for the DB file, but the registry can point to previous
		// installs if uninstall wasn't done properly.
		// Since the games DB file is considered part of pcsx2.exe itself, look for it at the exe folder
		//   regardless of any other settings.

		// Note 1: Portable setup didn't suffer from this as install folder pointed already to the exe folder in portable.
		// Note 2: Other folders are either configurable (plugins, memcards, etc) or create their content automatically (inis)
		//           So the games DB was really the only one that suffers from residues of prior installs.

		//wxDirName dir = InstallFolder;
		//wxDirName dir = (wxDirName)wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();
		//file = ( dir + file ).GetFullPath();
	}


	if (!wxFileExists(file))
	{
		Console.Error(L"(GameDB) Database Not Found! [%s]", WX_STR(file));
		return *this;
	}

	u64 qpc_Start = GetCPUTicks();
	YamlGameDatabaseImpl gameDb = YamlGameDatabaseImpl();
	gameDb.initDatabase(std::string(file));
	//GameDatabaseSchema::GameEntry gameEntry = gameDb.findGame("GUST-00009");
	u64 qpc_end = GetCPUTicks();

	Console.WriteLn("(GameDB) TODO games on record (loaded in %ums)",
					(u32)(((qpc_end - qpc_Start) * 1000) / GetTickFrequency()));

	return *this;
}

AppGameDatabase* Pcsx2App::GetGameDatabase()
{
	pxAppResources& res(GetResourceCache());

	ScopedLock lock(m_mtx_LoadingGameDB);
	if (!res.GameDB)
	{
		res.GameDB = std::make_unique<AppGameDatabase>();
		res.GameDB->LoadFromFile();
	}
	return res.GameDB.get();
}

IGameDatabase* AppHost_GetGameDatabase()
{
	return wxGetApp().GetGameDatabase();
}

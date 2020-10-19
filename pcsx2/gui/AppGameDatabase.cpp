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

// TODO - check that this is being threaded properly, remove from 
AppGameDatabase& AppGameDatabase::LoadFromFile(const std::string& _file)
{
	fs::path gameDbPath = Path::Combine(Path::GetExecutableDirectory(), fs::path(_file));

	if (!folderUtils.DoesExist(gameDbPath))
	{
		wxString temp = gameDbPath.c_str();
		Console.Error(L"(GameDB) Database Not Found! [%s]", WX_STR(temp));
		return *this;
	}

	u64 qpc_Start = GetCPUTicks();
	YamlGameDatabaseImpl gameDb = YamlGameDatabaseImpl();

	if (!gameDb.initDatabase(std::string(gameDbPath)))
	{
		Console.Error(L"(GameDB) Database could not be loaded successfully");
		return *this;
	}

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

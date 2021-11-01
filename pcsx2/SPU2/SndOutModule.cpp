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

#include "SndOutModule.h"

#include "SndOut_Null.h"
#include "SndOut_Portaudio.h"


SndOutModule* NullOut = new NullOutModule();
SndOutModule* PortaudioOut = new PortaudioOutModule();


SndOutModule* mods[] =
	{
		NullOut,
#ifdef _MSC_VER
		XAudio2Out,
#endif
#if defined(SPU2X_PORTAUDIO)
		PortaudioOut,
#endif
#if defined(__linux__) || defined(__APPLE__)
		SDLOut,
#endif
		nullptr // signals the end of our list
};

int FindOutputModuleById(const std::string omodid)
{
	int modcnt = 0;
	while (mods[modcnt] != nullptr)
	{
		if (mods[modcnt]->GetIdent() == omodid)
			break;
		++modcnt;
	}
	return modcnt;
}
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

#include "wx/string.h"
#include "common/Pcsx2Types.h"

class SndOutModule
{
public:
	// Virtual destructor, because it helps fight C+++ funny-business.
	virtual ~SndOutModule() {}

	// Returns a unique identification string for this driver.
	// (usually just matches the driver's cpp filename)
	virtual const std::string GetIdent() const = 0;

	// Returns the long name / description for this driver.
	// (for use in configuration screen)
	virtual const std::string GetLongName() const = 0;

	virtual s32 Init() = 0;
	virtual void Close() = 0;
	virtual s32 Test() const = 0;

	// Loads settings from the INI file for this driver
	virtual void ReadSettings() = 0;

	// Set output API for this driver
	virtual void SetApiSettings(wxString api) = 0;

	// Saves settings to the INI file for this driver
	virtual void WriteSettings() const = 0;

	// Returns the number of empty samples in the output buffer.
	// (which is effectively the amount of data played since the last update)
	virtual int GetEmptySampleCount() = 0;
};

extern int FindOutputModuleById(const std::string omodid);

extern SndOutModule* NullOut;
#ifdef _MSC_VER
//internal
extern SndOutModule* XAudio2Out;
#endif
#if defined(SPU2X_PORTAUDIO)
extern SndOutModule* PortaudioOut;
#endif
extern SndOutModule* const SDLOut;
extern SndOutModule* mods[];
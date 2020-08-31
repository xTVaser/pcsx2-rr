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

#include "InputRecordingFile.h"


#ifndef DISABLE_RECORDING
enum INPUT_RECORDING_MODE
{
	INPUT_RECORDING_MODE_NONE,
	INPUT_RECORDING_MODE_RECORD,
	INPUT_RECORDING_MODE_REPLAY,
};

class InputRecording
{
public:
	void controllerInterrupt(const u8 data, const u8 port, const u16 bufCount, u8 (&buf)[512]);

	void RecordModeToggle();

	INPUT_RECORDING_MODE getModeState() const noexcept;
	InputRecordingFile& getInputRecordingData() noexcept;
	bool isInterruptFrame() const noexcept;

	void stop();
	bool create(const wxString filename, const bool fromSaveState, const wxString authorName);
	bool play(wxString filename);
	bool loadFirstFrame();

private:
	InputRecordingFile m_InputRecordingData;
	INPUT_RECORDING_MODE m_state = INPUT_RECORDING_MODE_NONE;
	bool m_fInterruptFrame = false;
	// Resolve the name and region of the game currently loaded using the GameDB
	// If the game cannot be found in the DB, the fallback is the ISO filename
	wxString resolveGameName();
};

extern InputRecording g_InputRecording;
#endif

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
#ifndef DISABLE_RECORDING

#include "Recording/InputRecordingFile.h"

enum INPUT_RECORDING_MODE
{
	INPUT_RECORDING_MODE_NONE,
	INPUT_RECORDING_MODE_RECORD,
	INPUT_RECORDING_MODE_REPLAY,
};

class InputRecording
{
public:
	InputRecording();

	void ControllerInterrupt(u8 const &data, u8 const &port, u8 const &slot, u16 const &bufCount, u8 buf[]);

	void RecordModeToggle();

	INPUT_RECORDING_MODE GetModeState();
	InputRecordingFile & GetInputRecordingData();

	void Stop();
	bool Create(wxString filename, bool fromSaveState, wxString authorName, bool (&slots)[2][4]);
	bool Play(wxString filename);

	void SetVirtualPadPtr(VirtualPad *ptr, const u8 port, const u8 slot);
	void SetMultitapBuffer(const bool port1, const bool port2);
	bool GetMultitapBuffer(const u8 port);

private:
	static const int CONTROLLER_PORT_ONE = 0;
	static const int CONTROLLER_PORT_TWO = 1;
	static const int CONTROLLER_SLOT_A = 0;
	static const int CONTROLLER_SLOT_B = 1;
	static const int CONTROLLER_SLOT_C = 2;
	static const int CONTROLLER_SLOT_D = 3;
	//Holds pre-recording multitap settings 
	bool multitap_Buffer[2];

	// 0x42 is the magic number to indicate the default controller read query
	// See - Lilypad.cpp::PADpoll - https://github.com/PCSX2/pcsx2/blob/v1.5.0-dev/plugins/LilyPad/LilyPad.cpp#L1193
	static const u8 READ_DATA_AND_VIBRATE_FIRST_BYTE = 0x42;
	// 0x5A is always the second byte in the buffer when the normal READ_DATA_AND_VIBRATE (0x42) query is executed.
	// See - LilyPad.cpp::PADpoll - https://github.com/PCSX2/pcsx2/blob/v1.5.0-dev/plugins/LilyPad/LilyPad.cpp#L1194
    static const u8 READ_DATA_AND_VIBRATE_SECOND_BYTE = 0x5A;


	InputRecordingFile InputRecordingData;
	INPUT_RECORDING_MODE state = INPUT_RECORDING_MODE_NONE;
	bool fInterruptFrame = false;

	// Controller Data
	PadData *padData[2][4];

	// VirtualPads
	VirtualPad *virtualPads[2][4];
};

extern InputRecording g_InputRecording;
static InputRecordingFile& g_InputRecordingData = g_InputRecording.GetInputRecordingData();
static InputRecordingHeader& g_InputRecordingHeader = g_InputRecording.GetInputRecordingData().GetHeader();

#endif

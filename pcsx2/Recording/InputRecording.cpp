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

#ifndef DISABLE_RECORDING

#include <vector>

#include "AppSaveStates.h"
#include "AppGameDatabase.h"
#include "DebugTools/Debug.h"
#include "Counters.h"

#include "Recording/InputRecording.h"
#include "Recording/RecordingControls.h"

InputRecording g_InputRecording;

// Tag and save framecount along with savestate
void SaveStateBase::InputRecordingFreeze()
{
	FreezeTag("InputRecording");
	Freeze(g_FrameCount);

#ifndef DISABLE_RECORDING
	if (g_FrameCount > 0 && IsLoading())
	{
		g_InputRecordingData.AddUndoCount();
	}
#endif
}

InputRecording::InputRecording()
{
	// NOTE - No multi-tap support, only two controllers
    padData[CONTROLLER_PORT_ONE][CONTROLLER_SLOT_A] = new PadData();
	padData[CONTROLLER_PORT_ONE][CONTROLLER_SLOT_B] = new PadData();
	padData[CONTROLLER_PORT_ONE][CONTROLLER_SLOT_C] = new PadData();
	padData[CONTROLLER_PORT_ONE][CONTROLLER_SLOT_D] = new PadData();
	padData[CONTROLLER_PORT_TWO][CONTROLLER_SLOT_A] = new PadData();
	padData[CONTROLLER_PORT_TWO][CONTROLLER_SLOT_B] = new PadData();
	padData[CONTROLLER_PORT_TWO][CONTROLLER_SLOT_C] = new PadData();
	padData[CONTROLLER_PORT_TWO][CONTROLLER_SLOT_D] = new PadData();
}

void InputRecording::SetVirtualPadPtr(VirtualPad *ptr, const u8 port, const u8 slot)
{
	virtualPads[port][slot] = ptr;
}

// Main func for handling controller input data
// - Called by Sio.cpp::sioWriteController
void InputRecording::ControllerInterrupt(u8 const &data, u8 const &port, u8 const &slot, u16 const &bufCount, u8 buf[])
{
	// We are only interested in reading from the buffer when it contains the controller data.
    // The first byte in the buffer is used to indicate this.
    // A flag is set(fInterruptFrame) so that future interrupts will either be processed or skipped.
	if (bufCount == 1)
	{
        fInterruptFrame = data == READ_DATA_AND_VIBRATE_FIRST_BYTE;
	}
	else if ( bufCount == 2 )
	{
        // This is an extra sanity check, if this byte is different, then we unset the flag
		// and skip processing the buffer for this particular interrupt routine.
        if (buf[bufCount] != READ_DATA_AND_VIBRATE_SECOND_BYTE)
		{
			fInterruptFrame = false;
		}
	}

	// Skip processing this buffer if:
	// - Due to the prior checks, this interrupt is not something we are interested in
	// - We have got hit the actual data, which starts at the 3rd byte in the buffer
	if (!fInterruptFrame || bufCount < 3)
	{
		return;
	}

	u8 &bufVal = buf[bufCount];
	// For convenience sake, since the controller data starts at bufCount=3, we treat that as the first index. 
	const u16 bufIndex = bufCount - 3;

	// If we are replaying a movie, there should be NO modifications to the inputs
	// Grab the byte from the movie file and overwrite whatever the [Virtual]Pad is inputting
	if (state == INPUT_RECORDING_MODE_REPLAY)
	{
		if (InputRecordingData.GetMaxFrame() <= g_FrameCount)
		{
			// Pause the emulation but the movie is not closed
			g_RecordingControls.Pause();
			return;
		}
		u8 tmp = 0;
		if (InputRecordingData.ReadKeyBuf(tmp, g_FrameCount, port, slot, bufIndex))
		{
			// Overwrite value originally provided by the PAD plugin
			bufVal = tmp;
		}
	}

	// Update controller data state for future VirtualPad / logging usage.
	padData[port][slot]->UpdateControllerData(bufIndex, bufVal);

	if (virtualPads[port][slot] && virtualPads[port][slot]->IsShown())
	{
		// If the VirtualPad updated the PadData, we have to update the buffer
		// before committing it to the recording / sending it to the game
		// - Do not do this if we are in replay mode!
        if (virtualPads[port][slot]->UpdateControllerData(bufIndex, padData[port][slot]) && state != INPUT_RECORDING_MODE_REPLAY)
		{
			bufVal = padData[port][slot]->PollControllerData(bufIndex);
		}
	}

	// If we have reached the end of the pad data, log it out
    if (bufIndex == PadData::END_INDEX_CONTROLLER_BUFFER) {
		padData[port][slot]->LogPadData(port, slot);
		// As well as re-render the virtual pad UI, if applicable
		// - Don't render if it's minimized
        if (virtualPads[port][slot] && virtualPads[port][slot]->IsShown() && !virtualPads[port][slot]->IsIconized())
		{
			virtualPads[port][slot]->Redraw();
		}
	}

	// Finally, commit the byte to the movie file if we are recording
	if (state == INPUT_RECORDING_MODE_RECORD)
	{
		InputRecordingData.UpdateFrameMax(g_FrameCount);
		InputRecordingData.WriteKeyBuf(g_FrameCount, port, slot, bufIndex, bufVal);
	}
}

// GUI Handler - Stop recording
void InputRecording::Stop()
{
	state = INPUT_RECORDING_MODE_NONE;
	if (InputRecordingData.Close())
	{
		recordingConLog(L"[REC]: InputRecording Recording Stopped.\n");
		for (u8 port = 0; port <= CONTROLLER_PORT_TWO; port++)
		{
			for (u8 slot = 0; slot <= CONTROLLER_SLOT_D; slot++)
			{
				if (InputRecordingData.ActiveSlot(port, slot))
					virtualPads[port][slot]->ClearReadOnlyMode();
			}
		}
	}
}

// GUI Handler - Start recording
bool InputRecording::Create(wxString FileName, bool fromSaveState, wxString authorName, bool (&slots)[2][4])
{
	// create
	if (!InputRecordingData.Open(FileName, true, fromSaveState))
	{
		return false;
	}
	// Set author name
	if (!authorName.IsEmpty())
	{
		InputRecordingData.GetHeader().SetAuthor(authorName);
	}
	// Set used slots
	InputRecordingData.SetSlots(slots);
	// Set Game Name
	// Code loosely taken from AppCoreThread.cpp to resolve the Game Name
	// Fallback is ISO name
	wxString gameName;
	const wxString gameKey(SysGetDiscID());
	if (!gameKey.IsEmpty())
	{
		if (IGameDatabase* GameDB = AppHost_GetGameDatabase())
		{
			Game_Data game;
			if (GameDB->findGame(game, gameKey))
			{
				gameName = game.getString("Name");
				gameName += L" (" + game.getString("Region") + L")";
			}
		}
	}
	InputRecordingData.GetHeader().SetGameName(!gameName.IsEmpty() ? gameName : Path::GetFilename(g_Conf->CurrentIso));
	InputRecordingData.WriteHeader();
	InputRecordingData.WriteSaveState();
	InputRecordingData.WriteSlots();
	InputRecordingData.WriteMaxFrame();
	InputRecordingData.WriteUndoCount();
	state = INPUT_RECORDING_MODE_RECORD;
    // Ensure active VirtualPads aren't set to readOnly mode
	for (u8 port = 0; port <= CONTROLLER_PORT_TWO; port++)
	{
		for (u8 slot = 0; slot <= CONTROLLER_SLOT_D; slot++)
		{
			if (InputRecordingData.ActiveSlot(port, slot))
				virtualPads[port][slot]->ClearReadOnlyMode();
		}
	}
	recordingConLog(wxString::Format(L"[REC]: Started new recording - [%s]\n", FileName));

	// In every case, we reset the g_FrameCount
	g_FrameCount = 0;
	return true;
}

// GUI Handler - Play a recording
bool InputRecording::Play(wxString FileName)
{
	if (state != INPUT_RECORDING_MODE_NONE)
		Stop();

	if (!InputRecordingData.Open(FileName, false, false))
	{
		return false;
	}
	if (!InputRecordingData.ReadHeaderAndCheck())
	{
		recordingConLog(L"[REC]: This file is not a correct InputRecording file.\n");
		InputRecordingData.Close();
		return false;
	}
	state = INPUT_RECORDING_MODE_REPLAY;
	// Set active VirtualPads to readOnly mode
	for (u8 port = 0; port <= CONTROLLER_PORT_TWO; port++)
	{
		for (u8 slot = 0; slot <= CONTROLLER_SLOT_D; slot++)
		{
			if (InputRecordingData.ActiveSlot(port, slot))
				virtualPads[port][slot]->ClearReadOnlyMode();
		}
	}
	// TODO - This should be converted to a single log statement (since we want to guarantee its grouped)
	recordingConLog(wxString::Format(L"[REC]: Replaying movie - [%s]\n", FileName));
	// Check ISO name
	if (!g_Conf->CurrentIso.IsEmpty())
	{
		if (Path::GetFilename(g_Conf->CurrentIso) != InputRecordingData.GetHeader().gameName)
		{
			recordingConLog(L"[REC]: Information on CD in Movie file is Different.\n");
		}
	}
	recordingConLog(wxString::Format(L"[REC]: Recording File Version: %d\n", InputRecordingData.GetHeader().version));
	recordingConLog(wxString::Format(L"[REC]: Associated Game Name / ISO Filename: %s\n", InputRecordingData.GetHeader().gameName));
	recordingConLog(wxString::Format(L"[REC]: Author: %s\n", InputRecordingData.GetHeader().author));
	recordingConLog(wxString::Format(L"[REC]: MaxFrame: %d\n", InputRecordingData.GetMaxFrame()));
	recordingConLog(wxString::Format(L"[REC]: UndoCount: %d\n", InputRecordingData.GetUndoCount()));
	if (InputRecordingData.GetHeader().version == 2)
	{
		recordingConLog(L"[REC]: Controllers Used:");
		for (u8 port = 0; port <= CONTROLLER_PORT_TWO; port++)
		{
			for (u8 slot = 0; slot <= CONTROLLER_SLOT_D; slot++)
			{
				if (InputRecordingData.ActiveSlot(port, slot))
					recordingConLog(wxString::Format(L" %d%c", port + 1, 'A' + slot));
			}
		}
		recordingConLog(L'\n');
	}
	return true;
}

// Keybind Handler - Toggle between recording input and not
void InputRecording::RecordModeToggle()
{
	if (state == INPUT_RECORDING_MODE_REPLAY)
	{
		state = INPUT_RECORDING_MODE_RECORD;
        // Set active VirtualPads to readOnly mode
		for (u8 port = 0; port <= CONTROLLER_PORT_TWO; port++)
		{
			for (u8 slot = 0; slot <= CONTROLLER_SLOT_D; slot++)
			{
				if (InputRecordingData.ActiveSlot(port, slot))
					virtualPads[port][slot]->ClearReadOnlyMode();
			}
		}
		recordingConLog("[REC]: Record mode ON.\n");
	}
	else if (state == INPUT_RECORDING_MODE_RECORD)
	{
		state = INPUT_RECORDING_MODE_REPLAY;
        // Set active VirtualPads to readOnly mode
		for (u8 port = 0; port <= CONTROLLER_PORT_TWO; port++)
		{
			for (u8 slot = 0; slot <= CONTROLLER_SLOT_D; slot++)
			{
				if (InputRecordingData.ActiveSlot(port, slot))
					virtualPads[port][slot]->SetReadOnlyMode();
			}
		}
		recordingConLog("[REC]: Replay mode ON.\n");
	}
}

INPUT_RECORDING_MODE InputRecording::GetModeState()
{
	return state;
}

InputRecordingFile & InputRecording::GetInputRecordingData()
{
	return InputRecordingData;
}

void InputRecording::SetMultitapBuffer(const bool port1, const bool port2)
{
	multitap_Buffer[0] = port1;
	multitap_Buffer[1] = port2;
}

bool InputRecording::GetMultitapBuffer(const u8 port)
{
	return multitap_Buffer[port];
}
#endif
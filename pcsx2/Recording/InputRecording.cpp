/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2019  PCSX2 Dev Team
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

#include "AppSaveStates.h"
#include "AppGameDatabase.h"
#include "Common.h"
#include "Counters.h"
#include "MemoryTypes.h"
#include "SaveState.h"

#include "InputRecording.h"
#include "Recording/RecordingControls.h"

#include <vector>


// Tag and save framecount along with savestate
void SaveStateBase::InputRecordingFreeze()
{
	FreezeTag("InputRecording");
	Freeze(g_FrameCount);

#ifndef DISABLE_RECORDING
	if (g_FrameCount > 0 && IsLoading())
	{
		g_InputRecording.getinputRecordingData().incrementUndoCount();
	}
#endif
}

#ifndef DISABLE_RECORDING
InputRecording g_InputRecording;

// Main func for handling controller input data
// - Called by Sio.cpp::sioWriteController
void InputRecording::controllerInterrupt(const u8 data, const u8 port, const u16 buf_count, u8(&buf)[512])
{
	// TODO - Multi-Tap Support
	// Only examine controllers 1 / 2
	if (port != 0 && port != 1)
	{
		return;
	}

	/*
		This appears to try to ensure that we are only paying attention
		to the frames that matter, the ones that are reading from
		the controller.

		See - Lilypad.cpp::PADpoll - https://github.com/PCSX2/pcsx2/blob/v1.5.0-dev/plugins/LilyPad/LilyPad.cpp#L1193
		0x42 is the magic number for the default read query
	*/
	if (buf_count == 1)
	{
		m_fInterrupt_frame = data == 0x42;
		if (!m_fInterrupt_frame)
		{
			return;
		}
	}
	else if (buf_count == 2)
	{
		/*
			See - LilyPad.cpp::PADpoll - https://github.com/PCSX2/pcsx2/blob/v1.5.0-dev/plugins/LilyPad/LilyPad.cpp#L1194
			0x5A is always the second byte in the buffer
			when the normal READ_DATA_AND_VIBRRATE (0x42)
			query is executed, this looks like a sanity check
		*/
		if (buf[buf_count] != 0x5A)
		{
			m_fInterrupt_frame = false;
			return;
		}
	}

	if (!m_fInterrupt_frame || m_state == INPUT_RECORDING_MODE_NONE
		// We do not want to record or save the first two
		// bytes in the data returned from LilyPad
		|| buf_count < 3)
	{
		return;
	}

	// Read or Write
	if (m_state == INPUT_RECORDING_MODE_RECORD)
	{
		m_InputRecordingData.setTotalFrames(g_FrameCount);
		m_InputRecordingData.writeKeyBuffer(g_FrameCount, port, buf_count - 3, buf[buf_count]);
	}
	else if (m_state == INPUT_RECORDING_MODE_REPLAY)
	{
		if (m_InputRecordingData.getTotalFrames() <= g_FrameCount)
		{
			// Pause the emulation but the movie is not closed
			g_RecordingControls.pause();
			return;
		}
		u8 tmp = 0;
		if (m_InputRecordingData.readKeyBuffer(tmp, g_FrameCount, port, buf_count - 3))
		{
			buf[buf_count] = tmp;
		}
	}
}


// GUI Handler - Stop recording
void InputRecording::stop()
{
	m_state = INPUT_RECORDING_MODE_NONE;
	if (m_InputRecordingData.close())
		recordingConLog(L"[REC]: InputRecording Recording Stopped.\n");
}

// GUI Handler - Start recording
bool InputRecording::create(const wxString FileName, const bool from_savestate, const wxString author_name)
{
	if (!m_InputRecordingData.openNew(FileName, from_savestate))
		return false;
	// Set emulator version
	m_InputRecordingData.getHeader().setEmulatorVersion();

	// Set author name
	if (!author_name.IsEmpty())
		m_InputRecordingData.getHeader().setAuthor(author_name);
	// Set Game Name
	m_InputRecordingData.getHeader().setGameName(resolveGameName());
	// Write header contents
	m_InputRecordingData.writeHeader();
	m_state = INPUT_RECORDING_MODE_RECORD;
	recordingConLog(wxString::Format(L"[REC]: Started new recording - [%s]\n", FileName));

	// In every case, we reset the g_FrameCount
	g_FrameCount = 0;
	return true;
}

// GUI Handler - Play a recording
bool InputRecording::play(const wxString filename)
{
	if (m_state != INPUT_RECORDING_MODE_NONE)
		stop();

	// Open the file and verify if it can be played
	if (!m_InputRecordingData.openExisting(filename))
		return false;
	if (!loadFirstFrame())
	{
		stop();
		return false;
	}

	// Check if the current game matches with the one used to make the original recording
	if (!g_Conf->CurrentIso.IsEmpty())
	{
		if (resolveGameName() != m_InputRecordingData.getGameName())
			recordingConLog(L"[REC]: Recording was possibly recorded on a different game.\n");
	}

	m_state = INPUT_RECORDING_MODE_REPLAY;
	recordingConLog(wxString::Format(L"[REC]: Replaying input recording - [%s]\n", m_InputRecordingData.getFilename()));
	recordingConLog(wxString::Format(L"[REC]: PCSX2 Version Used: %s\n", m_InputRecordingData.getEmulatorVersion()));
	recordingConLog(wxString::Format(L"[REC]: Recording File Version: %d\n", m_InputRecordingData.getP2M2Version()));
	recordingConLog(wxString::Format(L"[REC]: Associated Game Name or ISO Filename: %s\n", m_InputRecordingData.getGameName()));
	recordingConLog(wxString::Format(L"[REC]: Author: %s\n", m_InputRecordingData.getAuthor()));
	recordingConLog(wxString::Format(L"[REC]: Total Frames: %d\n", m_InputRecordingData.getTotalFrames()));
	recordingConLog(wxString::Format(L"[REC]: Undo Count: %d\n", m_InputRecordingData.getUndoCount()));
	return true;
}

// Starts the recording at frame 0 either by loading the accompanying savestate or restarting emulation
bool InputRecording::loadFirstFrame()
{
	if (m_InputRecordingData.fromSaveState())
	{
		if (CoreThread.IsOpen())
		{
			if (wxFileExists(m_InputRecordingData.getFilename() + "_SaveState.p2s"))
			{
				StateCopy_LoadFromFile(m_InputRecordingData.getFilename() + "_SaveState.p2s");
				return true;
			}
			recordingConLog(wxString::Format("[REC]: Could not locate savestate file at location - %s_SaveState.p2s\n", m_InputRecordingData.getFilename()));
		}
		else
			recordingConLog(L"[REC]: Game is not open, cannot load the save-m_state accompanying the current recording.\n");
		return false;
	}
	sApp.SysExecute();
	return true;
}

wxString InputRecording::resolveGameName()
{
	// Code loosely taken from AppCoreThread::_ApplySettings to resolve the Game Name
	wxString game_name;
	const wxString game_key(SysGetDiscID());
	if (!game_key.IsEmpty())
	{
		if (IGameDatabase* game_database = AppHost_GetGameDatabase())
		{
			Game_Data game;
			if (game_database->findGame(game, game_key))
			{
				game_name = game.getString("Name");
				game_name += L" (" + game.getString("Region") + L")";
			}
		}
	}
	return !game_name.IsEmpty() ? game_name : Path::GetFilename(g_Conf->CurrentIso);
}

// Keybind Handler - Toggle between recording input and not
void InputRecording::recordModeToggle()
{
	if (m_state == INPUT_RECORDING_MODE_REPLAY)
	{
		m_state = INPUT_RECORDING_MODE_RECORD;
		recordingConLog("[REC]: Record mode ON.\n");
	}
	else if (m_state == INPUT_RECORDING_MODE_RECORD)
	{
		m_state = INPUT_RECORDING_MODE_REPLAY;
		recordingConLog("[REC]: Replay mode ON.\n");
	}
}

INPUT_RECORDING_MODE InputRecording::getModeState() const noexcept
{
	return m_state;
}

InputRecordingFile& InputRecording::getinputRecordingData() noexcept
{
	return m_InputRecordingData;
}

bool InputRecording::isInterruptFrame() const noexcept
{
	return m_fInterrupt_frame;
}
#endif

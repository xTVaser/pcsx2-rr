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
#include "ConsoleLogger.h"

#include "InputRecording.h"
#include "RecordingControls.h"


// Tag and save framecount along with savestate
void SaveStateBase::InputRecordingFreeze()
{
	FreezeTag("InputRecording");
	Freeze(g_FrameCount);

#ifndef DISABLE_RECORDING
	if (g_FrameCount > 0 && IsLoading())
		g_InputRecording.getInputRecordingData().incrementUndoCount();
#endif
}

#ifndef DISABLE_RECORDING
InputRecording g_InputRecording;

// Main func for handling controller input data
// - Called by Sio.cpp::sioWriteController
void InputRecording::controllerInterrupt(const u8 data, const u8 port, const u16 bufCount, u8 (&buf)[512])
{
	// TODO - Multi-Tap Support
	// Only examine controllers 1 / 2

	/*
		This appears to try to ensure that we are only paying attention
		to the frames that matter, the ones that are reading from
		the controller.

		See - Lilypad.cpp::PADpoll - https://github.com/PCSX2/pcsx2/blob/v1.5.0-dev/plugins/LilyPad/LilyPad.cpp#L1193
		0x42 is the magic number for the default read query
	*/
	if (bufCount == 1)
	{
		m_fInterruptFrame = data == 0x42;
		if (!m_fInterruptFrame)
			return;
	}
	else if (bufCount == 2)
	{
		/*
			See - LilyPad.cpp::PADpoll - https://github.com/PCSX2/pcsx2/blob/v1.5.0-dev/plugins/LilyPad/LilyPad.cpp#L1194
			0x5A is always the second byte in the buffer
			when the normal READ_DATA_AND_VIBRRATE (0x42)
			query is executed, this looks like a sanity check
		*/
		if (buf[bufCount] != 0x5A)
		{
			m_fInterruptFrame = false;
			return;
		}
	}

	if (!m_fInterruptFrame || m_state == INPUT_RECORDING_MODE_NONE
		// We do not want to record or save the first two
		// bytes in the data returned from LilyPad
		|| bufCount < 3)
	{
		return;
	}

	// Read or Write
	if (m_state == INPUT_RECORDING_MODE_RECORD)
	{
		m_InputRecordingData.setTotalFrames(g_FrameCount);
		m_InputRecordingData.writeKeyBuffer(buf[bufCount], g_FrameCount, bufCount - 3);
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
		if (m_InputRecordingData.readKeyBuffer(tmp, g_FrameCount, bufCount - 3))
			buf[bufCount] = tmp;
	}
}


// GUI Handler - Stop recording
void InputRecording::stop()
{
	m_state = INPUT_RECORDING_MODE_NONE;
	if (m_InputRecordingData.close())
		recordingConLog(L"[REC]: Input Recording Stopped.\n");
}

// GUI Handler - Start recording
bool InputRecording::create(wxString FileName, bool fromSaveState, wxString authorName)
{
	if (!m_InputRecordingData.openNew(FileName, fromSaveState))
		return false;

	m_InputRecordingData.getHeader().setEmulatorVersion();
	if (!authorName.IsEmpty())
		m_InputRecordingData.getHeader().setAuthor(authorName);
	m_InputRecordingData.getHeader().setGameName(resolveGameName());
	m_InputRecordingData.writeHeader();

	m_state = INPUT_RECORDING_MODE_RECORD;
	recordingConLog(wxString::Format(L"[REC]: Started new recording - [%s]\n", FileName));

	// In every case, we reset the g_FrameCount
	g_FrameCount = 0;
	return true;
}

// GUI Handler - Play a recording
bool InputRecording::play(wxString filename)
{
	if (m_state != INPUT_RECORDING_MODE_NONE)
		stop();

	// Open the file and verify if it can be played
	if (!m_InputRecordingData.openExisting(filename))
		return false;

	// Check if the current game matches with the one used to make the original recording
	if (!g_Conf->CurrentIso.IsEmpty())
		if (resolveGameName() != m_InputRecordingData.getGameName())
			recordingConLog(L"[REC]: Recording was possibly recorded on a different game.\n");
	m_state = INPUT_RECORDING_MODE_REPLAY;
	recordingConLog(wxString::Format(L"[REC]: Replaying input recording - [%s]\n", m_InputRecordingData.getFilename()));
	recordingConLog(wxString::Format(L"[REC]: PCSX2 Version Used: %s\n", m_InputRecordingData.getEmulatorVersion()));
	recordingConLog(wxString::Format(L"[REC]: Recording File Version: %d\n", m_InputRecordingData.getP2M2Version()));
	recordingConLog(wxString::Format(L"[REC]: Associated Game Name or ISO Filename: %s\n", m_InputRecordingData.getGameName()));
	recordingConLog(wxString::Format(L"[REC]: Author: %s\n", m_InputRecordingData.getAuthor()));
	recordingConLog(wxString::Format(L"[REC]: Total Frames: %d\n", m_InputRecordingData.getTotalFrames()));
	recordingConLog(wxString::Format(L"[REC]: Undo Count: %d\n", m_InputRecordingData.getUndoCount()));

	if (!loadFirstFrame())
	{
		stop();
		return false;
	}
	return true;
}

// Starts the recording at frame 0 either by loading the accompanying savestate or restarting emulation
// TODO: fix g_framecount value when loading a savestate (fix-frame-handling branch)
// Also TODO: Attempt to move this function out of InputRecording, similarly to InputRecordingFreeze(), due to OSDlog
bool InputRecording::loadFirstFrame()
{
	if (m_state != INPUT_RECORDING_MODE_NONE)
	{
		if (m_InputRecordingData.fromSaveState())
		{
			if (CoreThread.IsOpen())
			{
				if (wxFileExists(m_InputRecordingData.getFilename() + "_SaveState.p2s"))
				{
					OSDlog(Color_StrongGreen, true, " > Loading %s_SaveState.p2s", m_InputRecordingData.getFilename().c_str());
					StateCopy_LoadFromFile(m_InputRecordingData.getFilename() + "_SaveState.p2s");
					return true;
				}
				recordingConLog(wxString::Format("[REC]: Could not locate savestate file at location - %s_SaveState.p2s\n", m_InputRecordingData.getFilename()));
			}
			else
				recordingConLog(L"[REC]: Game is not open, cannot load the savestate accompanying the current recording.\n");
		}
		else
		{
			OSDlog(Color_StrongGreen, true, " > Resetting Emulation");
			sApp.SysExecute();
			return true;
		}
	}
	return false;
}

wxString InputRecording::resolveGameName()
{
	// Code loosely taken from AppCoreThread::_ApplySettings to resolve the Game Name
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
	return !gameName.IsEmpty() ? gameName : Path::GetFilename(g_Conf->CurrentIso);
}

// Keybind Handler - Toggle between recording input and not
void InputRecording::RecordModeToggle()
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

InputRecordingFile& InputRecording::getInputRecordingData() noexcept
{
	return m_InputRecordingData;
}

bool InputRecording::isInterruptFrame() const noexcept
{
	return m_fInterruptFrame;
}
#endif

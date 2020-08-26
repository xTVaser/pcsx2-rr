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

#include "App.h"
#include "Common.h"
#include "Counters.h"
#include "MainFrame.h"
#include "MemoryTypes.h"

#include "InputRecordingFile.h"

#ifndef DISABLE_RECORDING

void InputRecordingFile::InputRecordingFileHeader::init()
{
	m_author.fill(0);
	m_gameName.fill(0);
	m_emulator.fill(0);
	m_undoCount = 0;
	m_totalFrames = 0;
}

void InputRecordingFile::InputRecordingFileHeader::setAuthor(wxString author)
{
	m_author.fill(0);
	strncpy(m_author.data(), author.c_str(), m_author.size() - 1);
}

void InputRecordingFile::InputRecordingFileHeader::setEmulatorVersion()
{
	const wxString emulator_version = wxString::Format("%s-%d.%d.%d", pxGetAppName().c_str(), PCSX2_VersionHi, PCSX2_VersionMid, PCSX2_VersionLo);
	m_emulator.fill(0);
	strncpy(m_emulator.data(), emulator_version.c_str(), m_emulator.size() - 1);
}

void InputRecordingFile::InputRecordingFileHeader::setGameName(wxString game_name)
{
	m_gameName.fill(0);
	strncpy(m_gameName.data(), game_name.c_str(), m_gameName.size() - 1);
}

u8 InputRecordingFile::getP2M2Version() const noexcept
{
	return m_header.m_p2m2_version;
}

const char* InputRecordingFile::getEmulatorVersion() const noexcept
{
	return m_header.m_emulator.data();
}

const char* InputRecordingFile::getAuthor() const noexcept
{
	return m_header.m_author.data();
}

const char* InputRecordingFile::getGameName() const noexcept
{
	return m_header.m_gameName.data();
}

unsigned long InputRecordingFile::getTotalFrames() const noexcept
{
	return m_header.m_totalFrames;
}

unsigned long InputRecordingFile::getUndoCount() const noexcept
{
	return m_header.m_undoCount;
}

bool InputRecordingFile::fromSaveState() const noexcept
{
	return m_header.m_savestate;
}

bool InputRecordingFile::close()
{
	if (m_recording_file != nullptr)
	{
		fclose(m_recording_file);
		m_recording_file = nullptr;
		m_filename.clear();
		return true;
	}
	return false;
}

const wxString& InputRecordingFile::getFilename() const noexcept
{
	return m_filename;
}

InputRecordingFile::InputRecordingFileHeader& InputRecordingFile::getHeader() noexcept
{
	return m_header;
}

void InputRecordingFile::incrementUndoCount()
{
	if (m_recording_file != nullptr)
	{
		m_header.m_undoCount++;
		fseek(m_recording_file, s_seekpoint_undo_count, SEEK_SET);
		fwrite(&m_header.m_undoCount, 4, 1, m_recording_file);
	}
}

bool InputRecordingFile::open(const wxString path, bool newRecording)
{
	if (newRecording)
	{
		if ((m_recording_file = wxFopen(path, L"wb+")) != nullptr)
		{
			m_filename = path;
			m_header.init();
			return true;
		}
	}
	else if ((m_recording_file = wxFopen(path, L"rb+")) != nullptr)
	{
		if (verifyRecordingFileHeader())
		{
			m_filename = path;
			return true;
		}
		close();
		recordingConLog(wxString::Format("[REC]: Input recording file m_header is invalid\n"));
		return false;
	}
	recordingConLog(wxString::Format("[REC]: Input Recording file opening failed. Error - %s\n", strerror(errno)));
	return false;
}

bool InputRecordingFile::openNew(const wxString path, bool from_savestate)
{
	if (from_savestate)
	{
		if (CoreThread.IsOpen())
		{
			m_header.m_savestate = true;
			if (wxFileExists(path + "_SaveState.p2s"))
				wxCopyFile(path + "_SaveState.p2s", path + "_SaveState.p2s.bak", false);
			StateCopy_SaveToFile(path + "_SaveState.p2s");
			return open(path, true);
		}
		recordingConLog(L"[REC]: Game is not open, aborting playing input recording which starts on a save-state.\n");
		return false;
	}
	else
	{
		m_header.m_savestate = false;
		if (open(path, true))
		{
			sApp.SysExecute();
			return true;
		}
		return false;
	}
	
}

bool InputRecordingFile::openExisting(const wxString path)
{
	return open(path, false);
}

bool InputRecordingFile::readKeyBuffer(u8& result, const uint& frame, const uint port, const uint buf_index) const
{
	if (m_recording_file != nullptr)
	{
		const u32 seek = getRecordingBlockSeekPoint(frame) + buf_index;
		if (fseek(m_recording_file, seek, SEEK_SET) == 0 && fread(&result, 1, 1, m_recording_file) == 1)
			return true;
	}
	return false;
}

void InputRecordingFile::setTotalFrames(unsigned long frame)
{
	if (m_recording_file != nullptr && m_header.m_totalFrames < frame)
	{
		m_header.m_totalFrames = frame;
		fseek(m_recording_file, s_seekpoint_total_frames, SEEK_SET);
		fwrite(&m_header.m_totalFrames, 4, 1, m_recording_file);
	}
}

bool InputRecordingFile::writeHeader() const
{
	if (m_recording_file != nullptr)
	{
		rewind(m_recording_file);
		// Note: These fwrite calls are separated due to struct memory padding.
		return fwrite(&m_header, s_seekpoint_total_frames, 1, m_recording_file) == 1 && fwrite(&m_header.m_totalFrames, 9, 1, m_recording_file) == 1;
	}
	return false;
}

bool InputRecordingFile::writeKeyBuffer(const uint& frame, const uint port, const uint buf_index, const u8& buf)
{
	if (m_recording_file != nullptr)
	{
		const u32 seek = getRecordingBlockSeekPoint(frame) + buf_index;
		if (fseek(m_recording_file, seek, SEEK_SET) == 0 && fwrite(&buf, 1, 1, m_recording_file) == 1)
		{
			fflush(m_recording_file);
			return true;
		}
	}
	return false;
}

long InputRecordingFile::getRecordingBlockSeekPoint(const long& frame) const noexcept
{
	return s_seekpoint_input_data + frame * m_input_bytes_per_frame;
}

bool InputRecordingFile::verifyRecordingFileHeader()
{
	if (m_recording_file == nullptr)
		return false;
	// Verify m_header contents
	rewind(m_recording_file);
	if (fread(&m_header, s_seekpoint_total_frames, 1, m_recording_file) != 1 || fread(&m_header.m_totalFrames, 9, 1, m_recording_file) != 1)
		return false;

	// Check for valid verison
	switch (getP2M2Version())
	{
	case 1: // Official V1.0
		m_padCount = 2;
		m_input_bytes_per_frame = 36;
		break;
	default:
		recordingConLog(wxString::Format("[REC]: Input recording file is not a supported version - %d\n", m_header.m_p2m2_version));
		return false;
	}
	return true;
}
#endif

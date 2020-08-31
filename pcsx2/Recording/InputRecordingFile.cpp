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

void InputRecordingFile::InputRecordingFileHeader::init() noexcept
{
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
	const wxString emulatorVersion = wxString::Format("%s-%d.%d.%d", pxGetAppName().c_str(), PCSX2_VersionHi, PCSX2_VersionMid, PCSX2_VersionLo);
	m_emulatorVersion.fill(0);
	strncpy(m_emulatorVersion.data(), emulatorVersion.c_str(), m_emulatorVersion.size() - 1);
}

void InputRecordingFile::InputRecordingFileHeader::setGameName(wxString gameName)
{
	m_gameName.fill(0);
	strncpy(m_gameName.data(), gameName.c_str(), m_gameName.size() - 1);
}

u8 InputRecordingFile::getP2M2Version() const noexcept
{
	return m_header.m_fileVersion;
}

const char* InputRecordingFile::getEmulatorVersion() const noexcept
{
	return m_header.m_emulatorVersion.data();
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
	if (m_recordingFile != nullptr)
	{
		m_header.m_undoCount++;
		fseek(m_recordingFile, s_seekpointUndoCount, SEEK_SET);
		fwrite(&m_header.m_undoCount, 4, 1, m_recordingFile);
	}
}

bool InputRecordingFile::close()
{
	if (m_recordingFile != nullptr)
	{
		fclose(m_recordingFile);
		m_recordingFile = nullptr;
		m_filename.clear();
		return true;
	}
	return false;
}

bool InputRecordingFile::open(const wxString path, bool newRecording)
{
	if (newRecording)
	{
		if ((m_recordingFile = wxFopen(path, L"wb+")) != nullptr)
		{
			m_filename = path;
			m_header.init();
			return true;
		}
	}
	else if ((m_recordingFile = wxFopen(path, L"rb+")) != nullptr)
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

bool InputRecordingFile::openNew(const wxString path, bool fromSavestate)
{
	if (fromSavestate)
	{
		if (CoreThread.IsOpen())
		{
			if (open(path, true))
			{
				m_header.m_savestate = true;
				if (wxFileExists(path + "_SaveState.p2s"))
				{
					wxString backupPath = path + "_SaveState.p2s.bak";
					while (wxFileExists(backupPath))
						backupPath += ".bak";
					wxCopyFile(path + "_SaveState.p2s", backupPath, false);
				}
				StateCopy_SaveToFile(path + "_SaveState.p2s");
				return true;
			}
		}
		else
			recordingConLog(L"[REC]: Game is not open, aborting playing input recording which starts on a save-state.\n");
	}
	else if (open(path, true))
	{
		m_header.m_savestate = false;
		sApp.SysExecute();
		return true;
	}
	return false;
}

bool InputRecordingFile::openExisting(const wxString path)
{
	return open(path, false);
}

bool InputRecordingFile::readKeyBuffer(u8& result, const uint frame, const uint bufIndex) const
{
	if (m_recordingFile != nullptr)
	{
		const u32 seek = getRecordingBlockSeekPoint(frame) + bufIndex;
		if (fseek(m_recordingFile, seek, SEEK_SET) == 0 && fread(&result, 1, 1, m_recordingFile) == 1)
			return true;
	}
	return false;
}

void InputRecordingFile::setTotalFrames(const unsigned long frame)
{
	if (m_recordingFile != nullptr && m_header.m_totalFrames < frame)
	{
		m_header.m_totalFrames = frame;
		fseek(m_recordingFile, s_seekpointTotalFrames, SEEK_SET);
		fwrite(&m_header.m_totalFrames, 4, 1, m_recordingFile);
	}
}

bool InputRecordingFile::writeHeader()
{
	if (m_recordingFile != nullptr)
	{
		rewind(m_recordingFile);
		// Note: These fwrite calls are separated due to struct memory padding.
		return fwrite(&m_header, s_seekpointTotalFrames, 1, m_recordingFile) == 1 && fwrite(&m_header.m_totalFrames, 9, 1, m_recordingFile) == 1;
	}
	return false;
}

bool InputRecordingFile::writeKeyBuffer(const u8 buf, const uint frame, const uint bufIndex) const
{
	if (m_recordingFile != nullptr)
	{
		const u32 seek = getRecordingBlockSeekPoint(frame) + bufIndex;
		if (fseek(m_recordingFile, seek, SEEK_SET) == 0 && fwrite(&buf, 1, 1, m_recordingFile) == 1)
		{
			fflush(m_recordingFile);
			return true;
		}
	}
	return false;
}

long InputRecordingFile::getRecordingBlockSeekPoint(const long frame) const noexcept
{
	return s_seekpointInputData + frame * m_inputBytesPerFrame;
}

bool InputRecordingFile::verifyRecordingFileHeader()
{
	if (m_recordingFile == nullptr)
		return false;
	// Verify header contents
	rewind(m_recordingFile);
	if (fread(&m_header, s_seekpointTotalFrames, 1, m_recordingFile) != 1 || fread(&m_header.m_totalFrames, 9, 1, m_recordingFile) != 1)
		return false;

	// Check for valid verison
	switch (getP2M2Version())
	{
		case 1: // Official V1.0
			m_padCount = 2;
			m_inputBytesPerFrame = 36;
			break;
		default:
			recordingConLog(wxString::Format("[REC]: Input recording file is not a supported version - %d\n", m_header.m_fileVersion));
			return false;
	}
	return true;
}
#endif

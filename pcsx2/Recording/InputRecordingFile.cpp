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

void InputRecordingFileHeader::Init()
{
	memset(author, 0, ArraySize(author));
	memset(gameName, 0, ArraySize(gameName));
}

void InputRecordingFileHeader::SetEmulatorVersion()
{
	wxString emuVersion = wxString::Format("%s-%d.%d.%d", pxGetAppName().c_str(), PCSX2_VersionHi, PCSX2_VersionMid, PCSX2_VersionLo);
	int max = ArraySize(emu) - 1;
	strncpy(emu, emuVersion.c_str(), max);
	emu[max] = 0;
}

void InputRecordingFileHeader::SetAuthor(wxString _author)
{
	int max = ArraySize(author) - 1;
	strncpy(author, _author.c_str(), max);
	author[max] = 0;
}

void InputRecordingFileHeader::SetGameName(wxString _gameName)
{
	int max = ArraySize(gameName) - 1;
	strncpy(gameName, _gameName.c_str(), max);
	gameName[max] = 0;
}

bool InputRecordingFile::Close()
{
	if (recordingFile != nullptr)
	{
		fclose(recordingFile);
		recordingFile = nullptr;
		filename = "";
		return true;
	}
	return false;
}

const wxString &InputRecordingFile::GetFilename()
{
	return filename;
}

InputRecordingFileHeader &InputRecordingFile::GetHeader()
{
	return header;
}

unsigned long &InputRecordingFile::GetTotalFrames()
{
	return header.totalFrames;
}

unsigned long &InputRecordingFile::GetUndoCount()
{
	return header.undoCount;
}

bool InputRecordingFile::FromSaveState()
{
	return header.savestate;
}

void InputRecordingFile::IncrementUndoCount()
{
	if (recordingFile != nullptr)
	{
		header.undoCount++;
		fseek(recordingFile, seekpointUndoCount, SEEK_SET);
		fwrite(&header.undoCount, 4, 1, recordingFile);
	}
}

bool InputRecordingFile::open(const wxString path, bool newRecording)
{
	if (newRecording)
	{
		if ((recordingFile = wxFopen(path, L"wb+")) != nullptr)
		{
			filename = path;
			header.Init();
			return true;
		}
	}
	else if ((recordingFile = wxFopen(path, L"rb+")) != nullptr)
	{
		if (verifyRecordingFileHeader())
		{
			filename = path;
			return true;
		}
		else
		{
			Close();
			recordingConLog(wxString::Format("[REC]: Input recording file header is invalid\n"));
			return false;
		}
	}
	recordingConLog(wxString::Format("[REC]: Input Recording file opening failed. Error - %s\n", strerror(errno)));
	return true;
}

bool InputRecordingFile::OpenNew(const wxString path, bool fromSaveState)
{
	if (fromSaveState)
	{
		if (CoreThread.IsOpen())
		{
			header.savestate = true;
			FILE* ssFileCheck = wxFopen(path + "_SaveState.p2s", "r");
			if (ssFileCheck != nullptr)
			{
				wxCopyFile(path + "_SaveState.p2s", path + "_SaveState.p2s.bak", false);
				fclose(ssFileCheck);
			}
			StateCopy_SaveToFile(path + "_SaveState.p2s");
			return open(path, true);
		}
		else
		{
			recordingConLog(L"[REC]: Game is not open, aborting playing input recording which starts on a save-state.\n");
			return false;
		}
	} 
	else`
	{
		header.savestate = false;
		if (open(path, true))
		{
			sApp.SysExecute();
			return true;
		}
		else
			return false;
	}
	
}

bool InputRecordingFile::OpenExisting(const wxString path)
{
	return open(path, false);
}

bool InputRecordingFile::ReadKeyBuffer(u8 &result, const uint &frame, const uint port, const uint bufIndex)
{
	if (recordingFile != nullptr)
	{
		const u32 seek = getRecordingBlockSeekPoint(frame) + bufIndex;
		if (fseek(recordingFile, seek, SEEK_SET) == 0 && fread(&result, 1, 1, recordingFile) == 1)
		{
			return true;
		}
	}
	return false;
}

void InputRecordingFile::SetTotalFrames(unsigned long frame)
{
	if (recordingFile != nullptr && header.totalFrames < frame)
	{
		header.totalFrames = frame;
		fseek(recordingFile, seekpointTotalFrames, SEEK_SET);
		fwrite(&header.totalFrames, 4, 1, recordingFile);
	}
}

bool InputRecordingFile::WriteHeader()
{
	if (recordingFile != nullptr)
	{
		rewind(recordingFile);
		// Note: These fwrite calls are separated due to struct memory padding.
		return fwrite(&header, seekpointTotalFrames, 1, recordingFile) == 1 && fwrite(&header.totalFrames, 9, 1, recordingFile) == 1;
	}
	else
	{
		return false;
	}
}

bool InputRecordingFile::WriteKeyBuffer(const uint &frame, const uint port, const uint bufIndex, const u8 &buf)
{
	if (recordingFile != nullptr)
	{
		const u32 seek = getRecordingBlockSeekPoint(frame) + bufIndex;
		if (fseek(recordingFile, seek, SEEK_SET) == 0 && fwrite(&buf, 1, 1, recordingFile) == 1)
		{
			fflush(recordingFile);
			return true;
		}
	}
	//Reaches this return statement if anything else fails
	return false;
}

long InputRecordingFile::getRecordingBlockSeekPoint(const long &frame)
{
	return seekpointInputData + frame * inputBytesPerFrame;
}

bool InputRecordingFile::verifyRecordingFileHeader()
{
	if (recordingFile == nullptr)
	{
		return false;
	}
	// Verify header contents
	rewind(recordingFile);
	if (fread(&header, seekpointTotalFrames, 1, recordingFile) != 1 || fread(&header.totalFrames, 9, 1, recordingFile) != 1)
	{
		return false;
	}

	// Check for valid verison
	switch (header.version)
	{
	case 1: // Official V1.0
		padCount = 2;
		inputBytesPerFrame = 36;
		break;
	default:
		recordingConLog(wxString::Format("[REC]: Input recording file is not a supported version - %d\n", header.version));
		return false;
	}
	return true;
}
#endif

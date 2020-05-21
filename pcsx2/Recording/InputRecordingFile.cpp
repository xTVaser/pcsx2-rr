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

#include "DebugTools/Debug.h"
#include "MainFrame.h"
#include "MemoryTypes.h"

#include "Recording/InputRecordingFile.h"

#ifndef DISABLE_RECORDING
u32 InputRecordingFile::GetBlockSeekPoint(const u32 & frame)
{
	return RecordingHeaderSize + frame * RecordingBlockSize;
}

// Inits the new (or existing) input recording file
bool InputRecordingFile::Open(const wxString path, bool fNewOpen, bool fromSaveState)
{
	Close();
	wxString mode = L"rb+";
	if (fNewOpen)
	{
		mode = L"wb+";
		MaxFrame = 0;
		UndoCount = 0;
		header.Init();
	}
	recordingFile = wxFopen(path, mode);
	if ( recordingFile == nullptr )
	{
		recordingConLog(wxString::Format("[REC]: Movie file opening failed. Error - %s\n", strerror(errno)));
		return false;
	}
	filename = path;

	if (fNewOpen)
	{
		if (fromSaveState)
		{
			savestate.fromSavestate = true;
			FILE* ssFileCheck = wxFopen(path + "_SaveState.p2s", "r");
			if (ssFileCheck != nullptr)
			{
				wxCopyFile(path + "_SaveState.p2s", path + "_SaveState.p2s.bak", false);
				fclose(ssFileCheck);
			}
			StateCopy_SaveToFile(path + "_SaveState.p2s");
		}
		else
		{
			sApp.SysExecute();
		}
	}
	return true;
}

// Gracefully close the current recording file
bool InputRecordingFile::Close()
{
	if (recordingFile == nullptr)
	{
		return false;
	}
	fclose(recordingFile);
	recordingFile = nullptr;
	filename = "";
	return true;
}

// Write controller input buffer to file (per frame)
bool InputRecordingFile::WriteKeyBuf(const u32 & frame, const u8 port, const u8 slot, const u16 bufIndex, const u8 & buf)
{
	if (recordingFile == nullptr)
	{
		return false;
	}

	u32 seek = GetBlockSeekPoint(frame) + bufIndex;
	for (u8 i = 0; i < 4 * port + slot; i++)
	{
		if (Slots[i / 4][i % 4])
		{
			seek += RecordingBlockBaseSize;
		}
	}

	if (fseek(recordingFile, seek, SEEK_SET) != 0
		|| fwrite(&buf, 1, 1, recordingFile) != 1)
	{
		return false;
	}

	fflush(recordingFile);
	return true;
}

// Read controller input buffer from file (per frame)
bool InputRecordingFile::ReadKeyBuf(u8 & result, const u32 & frame, const u8 port, const u8 slot, const u16 bufIndex)
{
	if (recordingFile == nullptr)
	{
		return false;
	}

	u32 seek = GetBlockSeekPoint(frame) + bufIndex;
	//Traverse like a 1D array
	for (u8 i = 0; i < 4 * port + slot; i++)
	{
		if (Slots[i / 4][i % 4])
		{
			seek += RecordingBlockBaseSize;
		}
	}

	if (fseek(recordingFile, seek, SEEK_SET) != 0)
	{
		return false;
	}
	if (fread(&result, 1, 1, recordingFile) != 1)
	{
		return false;
	}

	return true;
}

// Verify header of recording file
bool InputRecordingFile::ReadHeaderAndCheck()
{
	if (recordingFile == nullptr)
	{
		return false;
	}
	rewind(recordingFile);
	if (fread(&header, sizeof(InputRecordingHeader), 1, recordingFile) != 1)
	{
		return false;
	}
	// Check for current verison
	switch (header.version) 
	{
	case 1: //Official version 1 with no slots written to file; Backwards compatibility
		Slots[0][0] = Slots[1][0] = true;
		//Format order: MaxFrame - > UndoCount - > Savestate
		if (fread(&MaxFrame, 4, 1, recordingFile) != 1
			|| fread(&UndoCount, 4, 1, recordingFile) != 1
			|| fread(&savestate.fromSavestate, sizeof(bool), 1, recordingFile) != 1)
		{
			return false;
		}
		RecordingSeekpointFrameMax = sizeof(InputRecordingHeader);
		RecordingSeekpointUndoCount = RecordingSeekpointFrameMax + 4;
		RecordingSeekpointSaveState = RecordingSeekpointUndoCount + 4;
		RecordingHeaderSize = sizeof(InputRecordingHeader) + 4 + 4 + 1;
		RecordingBlockSize = 36;
	case 2: //Version 2
		//Format order: Slots - > Savestate - > MaxFrame - > UndoCount
		if (fread(&savestate.fromSavestate, sizeof(bool), 1, recordingFile) != 1
			|| fread(&Slots, 8, 1, recordingFile) != 1
			|| fread(&MaxFrame, 4, 1, recordingFile) != 1
			|| fread(&UndoCount, 4, 1, recordingFile) != 1)
		{
			return false;
		}
		RecordingSeekpointSaveState = sizeof(InputRecordingHeader);
		RecordingSeekpointFrameMax = RecordingSeekpointSlots + 8;
		RecordingSeekpointUndoCount = RecordingSeekpointFrameMax + 4;
		RecordingHeaderSize = sizeof(InputRecordingHeader) + 8 + 1 + 4 + 4;
		RecordingBlockSize = 0;
		for (u8 port = 0; port < 2; port++)
		{
			for (u8 slot = 0; slot < 4; slot++)
			{
				if (Slots[port][slot])
					RecordingBlockSize += 18;
			}
		}
		if (RecordingBlockSize == 0)
		{
			recordingConLog(wxString::Format("[REC]: Input Recording File must have at least 1 controller\n"));
			return false;
		}
		break;
	default:
		recordingConLog(wxString::Format("[REC]: Input recording file is not a supported version - %d\n", header.version));
		return false;
	}
	if (savestate.fromSavestate)
	{
		FILE* ssFileCheck = wxFopen(filename + "_SaveState.p2s", "r");
		if (ssFileCheck == nullptr)
		{
			recordingConLog(wxString::Format("[REC]: Could not locate savestate file at location - %s\n", filename + "_SaveState.p2s"));
			return false;
		}
		fclose(ssFileCheck);
		StateCopy_LoadFromFile(filename + "_SaveState.p2s");
	}
	else
	{
		sApp.SysExecute();
	}
	return true;
}

bool InputRecordingFile::WriteHeader()
{
	if (recordingFile == nullptr)
	{
		return false;
	}
	rewind(recordingFile);
	if (fwrite(&header, sizeof(InputRecordingHeader), 1, recordingFile) != 1)
	{
		return false;
	}
	return true;
}

// Write savestate flag to file
bool InputRecordingFile::WriteSaveState() {
	if (recordingFile == nullptr)
	{
		return false;
	}

	fseek(recordingFile, RecordingSeekpointSaveState, SEEK_SET);
	if (fwrite(&savestate.fromSavestate, sizeof(bool), 1, recordingFile) != 1)
	{
		return false;
	}

	return true;
}

bool InputRecordingFile::WriteSlots()
{
	if (recordingFile == nullptr)
	{
		return false;
	}
	fseek(recordingFile, RecordingSeekpointSlots, SEEK_SET);
	if (fwrite(&Slots, 1, 8, recordingFile) != 1)
	{
		return false;
	}
	return true;
}

bool InputRecordingFile::WriteMaxFrame()
{
	if (recordingFile == nullptr)
	{
		return false;
	}
	fseek(recordingFile, RecordingSeekpointFrameMax, SEEK_SET);
	if (fwrite(&MaxFrame, 4, 1, recordingFile) != 1)
	{
		return false;
	}
	return true;
}

void InputRecordingFile::UpdateFrameMax(u32 frame)
{
	if (recordingFile == nullptr || MaxFrame >= frame)
	{
		return;
	}
	MaxFrame = frame;
	fseek(recordingFile, RecordingSeekpointFrameMax, SEEK_SET);
	fwrite(&MaxFrame, 4, 1, recordingFile);
}

bool InputRecordingFile::WriteUndoCount()
{
	if (recordingFile == nullptr)
	{
		return false;
	}
	fseek(recordingFile, RecordingSeekpointUndoCount, SEEK_SET);
	if (fwrite(&UndoCount, 4, 1, recordingFile) != 1)
	{
		return false;
	}
	return true;
}

void InputRecordingFile::AddUndoCount()
{
	UndoCount++;
	if (recordingFile == nullptr)
	{
		return;
	}
	fseek(recordingFile, RecordingSeekpointUndoCount, SEEK_SET);
	fwrite(&UndoCount, 4, 1, recordingFile);
}

bool InputRecordingFile::ActivePort(u8 port)
{
	return Slots[port][0] || Slots[port][1] || Slots[port][2] || Slots[port][3];
}

bool InputRecordingFile::ActiveMultitap(u8 port)
{
	return Slots[port][1] || Slots[port][2] || Slots[port][3];
}

bool InputRecordingFile::ActiveSlot(u8 port, u8 slot)
{
	return Slots[port][slot];
}

void InputRecordingHeader::SetAuthor(wxString _author)
{
	int max = ArraySize(author) - 1;
	strncpy(author, _author.c_str(), max);
	author[max] = 0;
}

void InputRecordingHeader::SetGameName(wxString _gameName)
{
	int max = ArraySize(gameName) - 1;
	strncpy(gameName, _gameName.c_str(), max);
	gameName[max] = 0;
}

void InputRecordingHeader::Init()
{
	memset(author, 0, ArraySize(author));
	memset(gameName, 0, ArraySize(gameName));
}

void InputRecordingFile::SetSlots(bool slots[2][4])
{
	RecordingBlockSize = 0;
	for (u8 port = 0; port < 2; port++)
	{
		for (u8 slot = 0; slot < 4; slot++)
		{
			Slots[port][slot] = slots[port][slot];
			RecordingBlockSize += 18 * Slots[port][slot];
		}
	}
}

InputRecordingHeader& InputRecordingFile::GetHeader()
{
	return header;
}

u32& InputRecordingFile::GetMaxFrame()
{
	return MaxFrame;
}

u32& InputRecordingFile::GetUndoCount()
{
	return UndoCount;
}

void InputRecordingFile::RetrieveSlots(bool (&slots)[2][4])
{
	memcpy(slots, Slots, 8);
}

const wxString & InputRecordingFile::GetFilename()
{
	return filename;
}
#endif

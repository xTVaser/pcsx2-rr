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

struct InputRecordingHeader
{
	u8 version = 2;
	char emu[50] = "PCSX2-1.5.X";
	char author[255] = "";
	char gameName[255] = "";

public:
	void SetAuthor(wxString author);
	void SetGameName(wxString cdrom);
	void Init();
};

// Contains info about the starting point of the movie
struct InputRecordingSavestate
{
	// Whether we start from the savestate or from power-on
	bool fromSavestate = false;
};

class InputRecordingFile
{
public:
	InputRecordingFile() {}
	~InputRecordingFile() { Close(); }

	// Movie File Manipulation
	bool Open(const wxString fn, bool fNewOpen, bool fromSaveState);
	bool Close();
	bool WriteKeyBuf(const u32 & frame, const u8 port, const u8 slot, const u16 bufIndex, const u8 & buf);
	bool ReadKeyBuf(u8 & result, const u32 & frame, const u8 port, const u8 slot, const u16 bufIndex);

	// Header
	InputRecordingHeader& GetHeader();
	u32& GetMaxFrame();
	u32& GetUndoCount();
	const wxString & GetFilename();
	void RetrieveSlots(bool (&slots)[2][4]);
	bool ActivePort(u8 port);
	bool ActiveMultitap(u8 port);
	bool ActiveSlot(u8 port, u8 slot);

	bool WriteHeader();
	bool WriteSaveState();
	bool WriteSlots();
	bool WriteMaxFrame();
	bool WriteUndoCount();

	bool ReadHeaderAndCheck();
	void UpdateFrameMax(u32 frame);
	void AddUndoCount();
	void SetSlots(bool slots[2][4]);

private:
	//Commented out as I'm not sure what the plan was/is for this
	//static const u16 RecordingSavestateHeaderSize = sizeof(bool);
	u16 RecordingSeekpointSaveState = sizeof(InputRecordingHeader);
	static const u16 RecordingSeekpointSlots = sizeof(InputRecordingHeader) + 1;
	u16 RecordingSeekpointFrameMax = RecordingSeekpointSlots + 8;
	u16 RecordingSeekpointUndoCount = RecordingSeekpointFrameMax + 4;
	static const u16 RecordingBlockBaseSize = 18;
	u16 RecordingBlockSize = RecordingBlockBaseSize;
	

	// Movie File
	FILE * recordingFile = NULL;
	wxString filename = "";
	u32 GetBlockSeekPoint(const u32& frame);

	// Header
	InputRecordingHeader header;
	bool Slots[2][4];
	InputRecordingSavestate savestate;
	u32 MaxFrame = 0;
	u32 UndoCount = 0;
	u16 RecordingHeaderSize = sizeof(InputRecordingHeader) + 1 + 8 + 4 + 4;
};
#endif

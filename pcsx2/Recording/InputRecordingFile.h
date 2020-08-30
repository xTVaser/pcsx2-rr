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

#pragma once

#include "System.h"

#include "PadData.h"

// NOTE / TODOs for Version 2
// - Move fromSavestate, undoCount, and total frames into the header

#ifndef DISABLE_RECORDING
struct InputRecordingFileHeader
{
	u8 version = 1;
	char emu[50] = "";
	char author[255] = "";
	char gameName[255] = "";
	unsigned long totalFrames = 0;
	unsigned long undoCount = 0;
	bool savestate = false;
public:
	void SetEmulatorVersion();
	void Init();
	void SetAuthor(wxString author);
	void SetGameName(wxString cdrom);
};

// Handles all operations on the input recording file
class InputRecordingFile
{
public:
	~InputRecordingFile() { Close(); }

	// Closes the underlying input recording file, writing the header and 
	// prepares for a possible new recording to be started
	bool Close();
	// Retrieve the input recording's filename (not the path)
	const wxString &GetFilename();
	// Retrieve the input recording's header which contains high-level metadata on the recording
	InputRecordingFileHeader &GetHeader();
	// The maximum number of frames, or in other words, the length of the recording
	unsigned long &GetTotalFrames();
	// The number of times a save-state has been loaded while recording this movie
	// this is also often referred to as a "re-record"
	unsigned long &GetUndoCount();
	// Whether or not this input recording starts by loading a save-state or by booting the game fresh
	bool FromSaveState();
	// Increment the number of undo actions and commit it to the recording file
	void IncrementUndoCount();
	// Open an existing recording file
	bool OpenExisting(const wxString path);
	// Create and open a brand new input recording, either starting from a save-state or from
	// booting the game
	bool OpenNew(const wxString path, bool fromSaveState);
	// Reads the current frame's input data from the file in order to intercept and overwrite
	// the current frame's value from the emulator
	bool ReadKeyBuffer(u8 &result, const uint &frame, const uint port, const uint bufIndex);
	// Updates the total frame counter and commit it to the recording file
	void SetTotalFrames(unsigned long frames);
	// Persist the input recording file header's current state to the file
	bool WriteHeader();
	// Writes the current frame's input data to the file so it can be replayed
	bool WriteKeyBuffer(const uint &frame, const uint port, const uint bufIndex, const u8 &buf);

private:
	static const int controllerInputBytes = 18;
	static const int seekpointInputData = 570U;
	static const int seekpointUndoCount = seekpointInputData - 5;
	static const int seekpointTotalFrames = seekpointUndoCount - 4;

	InputRecordingFileHeader header;
	wxString filename = "";
	FILE* recordingFile = NULL;

	// TODO: Implementation of V2.0 (w/ multitap)
	// Values placed separate from the above values in preparation
	static const int padCount = 2;
	static const int inputBytesPerFrame = controllerInputBytes * padCount;

	// Calculates the position of the current frame in the input recording
	long getRecordingBlockSeekPoint(const long& frame);
	bool open(const wxString path, bool newRecording);
	bool verifyRecordingFileHeader();
};
#endif

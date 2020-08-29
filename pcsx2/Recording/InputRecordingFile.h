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
#include <array>

#include "PadData.h"

// NOTE / TODOs for Version 2
// - Move fromSavestate, undoCount, and total frames into the header

#ifndef DISABLE_RECORDING
// Handles all operations on the input recording file
class InputRecordingFile
{
private:
	struct InputRecordingFileHeader
	{
		friend class InputRecordingFile;
		void init();
		void setEmulatorVersion();
		void setAuthor(wxString author);
		void setGameName(wxString cdrom);
	private:
		u8 m_fileVersion = 1;
		std::array<char, 50> m_emulatorVersion;
		std::array<char, 255> m_author;
		std::array<char, 255> m_gameName;
		unsigned long m_totalFrames = 0;
		unsigned long m_undoCount = 0;
		bool m_savestate = false;
	};
public:
	u8 getP2M2Version() const noexcept;
	// The version of PCSX2 that the recording originated from
	const char* getEmulatorVersion() const noexcept;
	const char* getAuthor() const noexcept;
	// The name of the game/iso that the file is paired with
	const char* getGameName() const noexcept;
	// The maximum number of frames, or in other words, the length of the recording
	unsigned long getTotalFrames() const noexcept;
	// The number of times a save-state has been loaded while recording this movie
	// this is also often referred to as a "re-record"
	unsigned long getUndoCount() const noexcept;
	// Whether or not this input recording starts by loading a save-state or by booting the game fresh
	bool fromSaveState() const noexcept;

	~InputRecordingFile() { close(); }

	// Closes the underlying input recording file, writing the header and 
	// prepares for a possible new recording to be started
	bool close();
	// Retrieve the input recording's filename (not the path)
	const wxString& getFilename() const noexcept;
	// Retrieve the input recording's header which contains high-level metadata on the recording
	InputRecordingFileHeader& getHeader() noexcept;
	// Increment the number of undo actions and commit it to the recording file
	void incrementUndoCount();
	// Open an existing recording file
	bool openExisting(const wxString path);
	// Create and open a brand new input recording, either starting from a save-state or from
	// booting the game
	bool openNew(const wxString path, bool fromSavestate);
	// Reads the current frame's input data from the file in order to intercept and overwrite
	// the current frame's value from the emulator
	bool readKeyBuffer(u8& result, const uint& frame, const uint port, const uint bufIndex) const;
	// Updates the total frame counter and commit it to the recording file
	void setTotalFrames(unsigned long frames);
	// Persist the input recording file header's current state to the file
	bool writeHeader() const;
	// Writes the current frame's input data to the file so it can be replayed
	bool writeKeyBuffer(const uint& frame, const uint port, const uint bufIndex, const u8& buf);

private:
	static const int s_controllerInputBytes = 18;
	static const int s_seekpointInputData = 570U;
	static const int s_seekpointUndoCount = s_seekpointInputData - 5;
	static const int s_seekpointTotalFrames = s_seekpointUndoCount - 4;

	InputRecordingFileHeader m_header;
	wxString m_filename = "";
	FILE* m_recordingFile = nullptr;

	// TODO: Implementation of V2.0 (w/ multitap)
	// Values placed separate from the above values in preparation
	int m_padCount;
	int m_inputBytesPerFrame;

	// Calculates the position of the current frame in the input recording
	long getRecordingBlockSeekPoint(const long& frame) const noexcept;
	bool open(const wxString path, bool newRecording);
	bool verifyRecordingFileHeader();
};
#endif

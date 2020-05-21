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

#include <wx/wx.h>
#include <wx/filepicker.h>


#ifndef DISABLE_RECORDING
enum MenuIds_New_Recording_Frame
{
	MenuIds_New_Recording_Frame_File = 0,
	MenuIds_New_Recording_Frame_Author,
	MenuIds_New_Recording_Frame_From,
	MenuIds_New_Recording_Frame_Slot_1A,
	MenuIds_New_Recording_Frame_Slot_1B,
	MenuIds_New_Recording_Frame_Slot_1C,
	MenuIds_New_Recording_Frame_Slot_1D,
	MenuIds_New_Recording_Frame_Slot_2A,
	MenuIds_New_Recording_Frame_Slot_2B,
	MenuIds_New_Recording_Frame_Slot_2C,
	MenuIds_New_Recording_Frame_Slot_2D
};

// The Dialog to pop-up when recording a new movie
class NewRecordingFrame : public wxDialog
{
public:
	NewRecordingFrame(wxWindow *parent);

	wxString GetFile() const;
	wxString GetAuthor() const;
	int GetFrom() const;
	void RetrieveSlots(bool (&slotBuf)[2][4]) const;

private:
	wxStaticText* m_empty;
	wxStaticText *m_fileLabel;
	wxFilePickerCtrl *m_filePicker;
	wxStaticText *m_authorLabel;
	wxTextCtrl *m_authorInput;
	wxChoice *m_fromChoice;
	wxStaticText* m_ControllerLabel;
	wxStaticText* m_SlotLabel_1;
	wxStaticText* m_SlotLabel_2;
	wxCheckBox* m_SlotCheck[2][4];
	wxStaticText* m_fromLabel;
	wxButton *m_startRecording;
	wxButton *m_cancelRecording;
};
#endif

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

#include "NewRecordingFrame.h"


#ifndef DISABLE_RECORDING
NewRecordingFrame::NewRecordingFrame(wxWindow *parent)
	: wxDialog(parent, wxID_ANY, "New Input Recording", wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP | wxCAPTION)
{
	wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _("panel"));
	m_empty = new wxStaticText(panel, wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);

	wxFlexGridSizer *fgs = new wxFlexGridSizer(10, 2, 20, 20);
	wxBoxSizer *container = new wxBoxSizer(wxVERTICAL);

	m_fileLabel = new wxStaticText(panel, wxID_ANY, _("File Path"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_authorLabel = new wxStaticText(panel, wxID_ANY, _("Author"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_ControllerLabel = new wxStaticText(panel, wxID_ANY, _("Controllers"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_SlotLabel_1 = new wxStaticText(panel, wxID_ANY, _("Port 1"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	m_SlotLabel_2 = new wxStaticText(panel, wxID_ANY, _("Port 2"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);

	m_filePicker = new wxFilePickerCtrl(panel, MenuIds_New_Recording_Frame_File, wxEmptyString, "File", L"p2m2 file(*.p2m2)|*.p2m2", wxDefaultPosition, wxDefaultSize, wxFLP_SAVE | wxFLP_OVERWRITE_PROMPT | wxFLP_USE_TEXTCTRL);
	m_authorInput = new wxTextCtrl(panel, MenuIds_New_Recording_Frame_Author, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	m_SlotCheck[0][0] = new wxCheckBox(panel, MenuIds_New_Recording_Frame_Slot_1A, _("1A"), wxDefaultPosition, wxDefaultSize);
	m_SlotCheck[1][0] = new wxCheckBox(panel, MenuIds_New_Recording_Frame_Slot_2A, _("2A"), wxDefaultPosition, wxDefaultSize);
	m_SlotCheck[0][1] = new wxCheckBox(panel, MenuIds_New_Recording_Frame_Slot_1B, _("1B"), wxDefaultPosition, wxDefaultSize);
	m_SlotCheck[1][1] = new wxCheckBox(panel, MenuIds_New_Recording_Frame_Slot_2B, _("2B"), wxDefaultPosition, wxDefaultSize);
	m_SlotCheck[0][2] = new wxCheckBox(panel, MenuIds_New_Recording_Frame_Slot_1C, _("1C"), wxDefaultPosition, wxDefaultSize);
	m_SlotCheck[1][2] = new wxCheckBox(panel, MenuIds_New_Recording_Frame_Slot_2C, _("2C"), wxDefaultPosition, wxDefaultSize);
	m_SlotCheck[0][3] = new wxCheckBox(panel, MenuIds_New_Recording_Frame_Slot_1D, _("1D"), wxDefaultPosition, wxDefaultSize);
	m_SlotCheck[1][3] = new wxCheckBox(panel, MenuIds_New_Recording_Frame_Slot_2D, _("2D"), wxDefaultPosition, wxDefaultSize);
	m_fromLabel = new wxStaticText(panel, wxID_ANY, _("Record From"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	wxArrayString choices;
	choices.Add("Current Frame");
	choices.Add("Power-On");
	m_fromChoice = new wxChoice(panel, MenuIds_New_Recording_Frame_From, wxDefaultPosition, wxDefaultSize, choices);
	m_fromChoice->SetSelection(0);

	m_startRecording = new wxButton(panel, wxID_OK, _("Ok"), wxDefaultPosition, wxDefaultSize);
	m_cancelRecording = new wxButton(panel, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize);

	fgs->Add(m_fileLabel, 1);
	fgs->Add(m_filePicker, 1);

	fgs->Add(m_authorLabel, 1);
	fgs->Add(m_authorInput, 1, wxEXPAND);

	fgs->Add(m_ControllerLabel, 2);
	fgs->Add(m_empty, 1); //Empty spot

	fgs->Add(m_SlotLabel_1, 1);
	fgs->Add(m_SlotLabel_2, 1);

	fgs->Add(m_SlotCheck[0][0], 1);
	fgs->Add(m_SlotCheck[1][0], 1);
	fgs->Add(m_SlotCheck[0][1], 1);
	fgs->Add(m_SlotCheck[1][1], 1);
	fgs->Add(m_SlotCheck[0][2], 1);
	fgs->Add(m_SlotCheck[1][2], 1);
	fgs->Add(m_SlotCheck[0][3], 1);
	fgs->Add(m_SlotCheck[1][3], 1);

	fgs->Add(m_fromLabel, 1);
	fgs->Add(m_fromChoice, 1, wxEXPAND);

	fgs->Add(m_startRecording, 1);
	fgs->Add(m_cancelRecording, 1);

	container->Add(fgs, 1, wxALL | wxEXPAND, 15);
	panel->SetSizer(container);
	panel->GetSizer()->Fit(this);
	Centre();
}

wxString NewRecordingFrame::GetFile() const
{
	wxString path = m_filePicker->GetPath();
	// wxWidget's removes the extension if it contains wildcards
	// on wxGTK https://trac.wxwidgets.org/ticket/15285
	if (!path.EndsWith(".p2m2")) {
		return wxString::Format("%s.p2m2", path);
	}
	return path;
}

wxString NewRecordingFrame::GetAuthor() const
{
	return m_authorInput->GetValue();
}

int NewRecordingFrame::GetFrom() const
{
	return m_fromChoice->GetSelection();
}

//Grabs slot values from the window and inserts into the array provided
void NewRecordingFrame::RetrieveSlots(bool (&slotBuf)[2][4]) const
{
	//Port 1
	slotBuf[0][0] = m_SlotCheck[0][0]->GetValue();
	slotBuf[0][1] = m_SlotCheck[0][1]->GetValue();
	slotBuf[0][2] = m_SlotCheck[0][2]->GetValue();
	slotBuf[0][3] = m_SlotCheck[0][3]->GetValue();
	//Port 2
	slotBuf[1][0] = m_SlotCheck[1][0]->GetValue();
	slotBuf[1][1] = m_SlotCheck[1][1]->GetValue();
	slotBuf[1][2] = m_SlotCheck[1][2]->GetValue();
	slotBuf[1][3] = m_SlotCheck[1][3]->GetValue();
}

#endif

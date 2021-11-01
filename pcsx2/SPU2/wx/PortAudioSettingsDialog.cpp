/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2021  PCSX2 Dev Team
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

#include "PortAudioSettingsDialog.h"

#include "SPU2/SndOutModule.h"
#include "SPU2/SndOut_Portaudio.h"


PortAudioSettingsDialog::PortAudioSettingsDialog(wxDialog* parent)
	: wxDialog(parent, wxID_ANY, "PortAudio Settings", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	// TODO - null check?
	m_portAudio = static_cast<PortaudioOutModule*>(PortaudioOut);

	// Sizers

	wxBoxSizer* sizerWrapper = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerContents = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* sizerSelectionRow = new wxBoxSizer(wxHORIZONTAL);
	wxStaticBoxSizer* sizerHostApi = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Host API")), wxVERTICAL);
	wxStaticBoxSizer* sizerDeviceName = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Device Name")), wxVERTICAL);
	wxBoxSizer* sizerExclusiveMode = new wxBoxSizer(wxHORIZONTAL);
	wxStaticBoxSizer* sizerLatency = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, wxT("Output Latency")), wxVERTICAL);
	wxBoxSizer* sizerManualLatency = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizerDialogBtns = new wxBoxSizer(wxHORIZONTAL);

	sizerWrapper->Add(sizerContents, 0, wxALL, 10);
	sizerContents->Add(sizerSelectionRow, 0, wxBOTTOM | wxEXPAND | wxTOP, 5);
	sizerSelectionRow->Add(sizerHostApi, 3, wxEXPAND, 0);

	// Components

	const wxString m_hostApiSelect_choices[] = {""};
	m_hostApiSelect = new wxComboBox(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0, m_hostApiSelect_choices, wxCB_DROPDOWN);
	sizerHostApi->Add(m_hostApiSelect, 0, wxEXPAND, 0);

	sizerSelectionRow->Add(sizerDeviceName, 6, wxEXPAND | wxLEFT, 5);
	const wxString m_deviceNameSelect_choices[] = {""};
	m_deviceNameSelect = new wxComboBox(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, 0, m_deviceNameSelect_choices, wxCB_DROPDOWN);
	sizerDeviceName->Add(m_deviceNameSelect, 0, wxEXPAND, 0);

	sizerContents->Add(sizerExclusiveMode, 0, wxBOTTOM | wxEXPAND | wxTOP, 5);
	m_wasapiExclusive = new wxCheckBox(this, wxID_ANY, wxT("WASAPI Exclusive Mode"));
	sizerExclusiveMode->Add(m_wasapiExclusive, 0, wxLEFT, 5);

	sizerContents->Add(sizerLatency, 0, wxBOTTOM | wxEXPAND | wxTOP, 5);
	m_latencySmallest = new wxRadioButton(this, wxID_ANY, wxT("Use smallest possible"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	m_latencySmallest->SetValue(true);
	sizerLatency->Add(m_latencySmallest, 0, wxBOTTOM | wxTOP, 5);

	sizerLatency->Add(sizerManualLatency, 1, wxBOTTOM | wxEXPAND, 5);
	m_latencyManual = new wxRadioButton(this, wxID_ANY, wxT("Custom latency value"));
	sizerManualLatency->Add(m_latencyManual, 0, wxEXPAND, 0);

	m_latencyManualSelect = new wxSlider(this, wxID_ANY, 20, 10, 200, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_LABELS);
	sizerManualLatency->Add(m_latencyManualSelect, 1, wxALIGN_CENTER_VERTICAL, 0);

	wxStaticText* m_warningLabel = new wxStaticText(this, wxID_ANY, wxT("Note:\n- Using the smallest possible latency may cause issues if the value is too small\n- Depending on hardware and drivers, the suggested manual latency might not be used.  Portaudio will choose the closest possible value."));
	sizerLatency->Add(m_warningLabel, 0, 0, 5);

	sizerContents->Add(sizerDialogBtns, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM | wxTOP, 5);

	m_confirmBtn = new wxButton(this, wxID_OK, wxEmptyString);
	m_confirmBtn->SetDefault();
	sizerDialogBtns->Add(m_confirmBtn, 0, wxRIGHT, 5);

	m_cancelBtn = new wxButton(this, wxID_CANCEL, wxEmptyString);
	sizerDialogBtns->Add(m_cancelBtn, 0, 0, 0);

	// Bindings
	m_confirmBtn->Bind(wxEVT_BUTTON, &PortAudioSettingsDialog::ConfirmDialog, this);

	// Initialize Dialog
	m_wasapiExclusive->SetValue(m_portAudio->m_WasapiExclusiveMode);
	if (m_portAudio->m_SuggestedLatencyMinimal)
	{
		m_latencySmallest->SetValue(true);
		m_latencyManualSelect->Enable(false);
	}
	else
	{
		m_latencyManual->SetValue(true);
		m_latencyManualSelect->SetValue(m_portAudio->m_SuggestedLatencyMS);
		m_latencyManualSelect->Enable(true);
	}

	SetSizer(sizerWrapper);
	sizerWrapper->Fit(this);
	Layout();
}

void PortAudioSettingsDialog::ConfirmDialog(wxCommandEvent& event)
{
	m_portAudio->m_WasapiExclusiveMode = m_wasapiExclusive->IsChecked();
	m_portAudio->m_SuggestedLatencyMinimal = m_latencySmallest->GetValue();
	m_portAudio->m_SuggestedLatencyMS = m_latencyManualSelect->GetValue();
	if (IsModal())
	{
		EndModal(wxID_OK);
	}
	else
	{
		SetReturnCode(wxID_OK);
		this->Show(false);
	}
}

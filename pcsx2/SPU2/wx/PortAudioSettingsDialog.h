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

#pragma once

#include "SPU2/SndOut_Portaudio.h"

#include <wx/wx.h>

class PortAudioSettingsDialog : public wxDialog
{
public:
	PortAudioSettingsDialog(wxDialog* parent);

private:
	PortaudioOutModule* m_portAudio;
protected:
	wxComboBox* m_hostApiSelect;
	wxComboBox* m_deviceNameSelect;
	wxCheckBox* m_wasapiExclusive;
	wxRadioButton* m_latencySmallest;
	wxRadioButton* m_latencyManual;
	wxSlider* m_latencyManualSelect;
	wxButton* m_confirmBtn;
	wxButton* m_cancelBtn;

	void ConfirmDialog(wxCommandEvent& event);
};
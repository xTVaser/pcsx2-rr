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

#include <wx/wx.h>
#include <wx/tglbtn.h>
#include <wx/spinctrl.h>

#include "Recording/PadData.h"

#ifndef DISABLE_RECORDING
class VirtualPad : public wxFrame
{
public:
	VirtualPad(wxWindow* parent, wxWindowID id, const wxString& title, int controllerPort, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE);

	bool Show(bool show = true) override;

private:
	void setProperties();
	void doLayout();

	int m_controllerPort;

	wxToggleButton* m_l2Button;
	wxSpinCtrl* m_l2ButtonPressure;
	wxToggleButton* m_l1Button;
	wxSpinCtrl* m_l1ButtonPressure;
	wxToggleButton* m_r2Button;
	wxSpinCtrl* m_r2ButtonPressure;
	wxToggleButton* m_r1Button;
	wxSpinCtrl* m_r1ButtonPressure;
	wxToggleButton* m_upButton;
	wxSpinCtrl* m_upButtonPressure;
	wxToggleButton* m_leftButton;
	wxSpinCtrl* m_leftButtonPressure;
	wxToggleButton* m_rightButton;
	wxSpinCtrl* m_rightButtonPressure;
	wxToggleButton* m_downButton;
	wxSpinCtrl* m_downButtonPressure;
	wxToggleButton* m_startButton;
	wxToggleButton* m_selectButton;
	wxToggleButton* m_triangleButton;
	wxSpinCtrl* m_triangleButtonPressure;
	wxToggleButton* m_squareButton;
	wxSpinCtrl* m_squareButtonPressure;
	wxToggleButton* m_circleButton;
	wxSpinCtrl* m_circleButtonPressure;
	wxToggleButton* m_crossButton;
	wxSpinCtrl* m_crossButtonPressure;
	wxSlider* m_leftAnalogXVal;
	wxSpinCtrl* m_leftAnalogXValPrecise;
	wxToggleButton* m_l3Button;
	wxSlider* m_leftAnalogYVal;
	wxSpinCtrl* m_leftAnalogYValPrecise;
	wxSlider* m_rightAnalogXVal;
	wxSpinCtrl* m_rightAnalogXValPrecise;
	wxToggleButton* m_r3Button;
	wxSlider* m_rightAnalogYVal;
	wxSpinCtrl* m_rightAnalogYValPrecise;

	wxToggleButton* m_buttons[16];
	int m_buttonsLength = 16;
	wxSpinCtrl* m_buttonsPressure[12];
	int m_buttonsPressureLength = 12;
	wxSlider* m_analogSliders[4];
	int m_analogSlidersLength = 4;
	wxSpinCtrl* m_analogVals[4];
	int m_analogValsLength = 4;

	void onClose(wxCloseEvent &event);
	void onButtonPress(wxCommandEvent &event);
	void onPressureChange(wxSpinEvent &event);
	void onAnalogValChange(wxSpinEvent &event);
	void onAnalogSliderChange(wxCommandEvent &event);
	// TODO - reset button

protected:
	wxDECLARE_EVENT_TABLE();
};
#endif

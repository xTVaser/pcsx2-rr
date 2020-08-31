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

#include "Common.h"

#include "Recording/VirtualPad.h"
#include "Recording/RecordingInputManager.h"

#ifndef DISABLE_RECORDING
wxBEGIN_EVENT_TABLE(VirtualPad, wxFrame)
	EVT_CLOSE(VirtualPad::onClose)
wxEND_EVENT_TABLE()

// TODO - Problems / Potential improvements:
// - The UI doesn't update to manual controller inputs and actually overrides the controller when opened (easily noticable with analog stick)
//   - This is less than ideal, but it's going to take a rather large / focused refactor, in it's current state the virtual pad does what it needs to do (precise inputs, frame by frame)
VirtualPad::VirtualPad(wxWindow* parent, wxWindowID id, const wxString& title, int m_controllerPort, const wxPoint& pos, const wxSize& size, long style) :
	wxFrame(parent, id, title, pos, size, wxDEFAULT_FRAME_STYLE)
{
	// Define components
	SetSize(wxSize(1000, 700));
	m_l2Button = new wxToggleButton(this, wxID_ANY, wxT("L2"));
	m_l2ButtonPressure = new wxSpinCtrl(this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 255);
	m_l1Button = new wxToggleButton(this, wxID_ANY, wxT("L1"));
	m_l1ButtonPressure = new wxSpinCtrl(this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 255);
	m_r2Button = new wxToggleButton(this, wxID_ANY, wxT("R2"));
	m_r2ButtonPressure = new wxSpinCtrl(this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 255);
	m_r1Button = new wxToggleButton(this, wxID_ANY, wxT("R1"));
	m_r1ButtonPressure = new wxSpinCtrl(this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 255);
	m_upButton = new wxToggleButton(this, wxID_ANY, wxT("Up"));
	m_upButtonPressure = new wxSpinCtrl(this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 255);
	m_leftButton = new wxToggleButton(this, wxID_ANY, wxT("Left"));
	m_leftButtonPressure = new wxSpinCtrl(this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 255);
	m_rightButton = new wxToggleButton(this, wxID_ANY, wxT("Right"));
	m_rightButtonPressure = new wxSpinCtrl(this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 255);
	m_downButton = new wxToggleButton(this, wxID_ANY, wxT("Down"));
	m_downButtonPressure = new wxSpinCtrl(this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 255);
	m_selectButton = new wxToggleButton(this, wxID_ANY, wxT("Select"));
	m_startButton = new wxToggleButton(this, wxID_ANY, wxT("Start"));
	m_triangleButton = new wxToggleButton(this, wxID_ANY, wxT("Triangle"));
	m_triangleButtonPressure = new wxSpinCtrl(this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 255);
	m_squareButton = new wxToggleButton(this, wxID_ANY, wxT("Square"));
	m_squareButtonPressure = new wxSpinCtrl(this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 255);
	m_circleButton = new wxToggleButton(this, wxID_ANY, wxT("Circle"));
	m_circleButtonPressure = new wxSpinCtrl(this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 255);
	m_crossButton = new wxToggleButton(this, wxID_ANY, wxT("Cross"));
	m_crossButtonPressure = new wxSpinCtrl(this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 255, 255);
	m_leftAnalogXVal = new wxSlider(this, wxID_ANY, 0, -127, 127);
	m_leftAnalogXValPrecise = new wxSpinCtrl(this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -127, 128);
	m_l3Button = new wxToggleButton(this, wxID_ANY, wxT("L3"));
	m_leftAnalogYVal = new wxSlider(this, wxID_ANY, 0, -127, 127, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL);
	m_leftAnalogYValPrecise = new wxSpinCtrl(this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -127, 128);
	m_rightAnalogXVal = new wxSlider(this, wxID_ANY, 0, -127, 127);
	m_rightAnalogXValPrecise = new wxSpinCtrl(this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -127, 128);
	m_r3Button = new wxToggleButton(this, wxID_ANY, wxT("R3"));
	m_rightAnalogYVal = new wxSlider(this, wxID_ANY, 0, -127, 127, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL);
	m_rightAnalogYValPrecise = new wxSpinCtrl(this, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -127, 128);

	// Initialize class members
	VirtualPad::m_controllerPort = m_controllerPort;

	// NOTE: Order MATTERS, these match enum defined in PadData.h
	wxToggleButton* tempButtons[16] = {
		// Pressure sensitive buttons
		m_upButton, m_rightButton, m_leftButton, m_downButton,
		m_crossButton, m_circleButton, m_squareButton, m_triangleButton,
		m_l1Button, m_l2Button, m_r1Button, m_r2Button,
		// Non-pressure sensitive buttons
		m_l3Button, m_r3Button,
		m_selectButton, m_startButton};
	std::copy(std::begin(tempButtons), std::end(tempButtons), std::begin(m_buttons));

	// NOTE: Order MATTERS, these match enum defined in PadData.h
	wxSpinCtrl* tempPressureButtons[16] = {
		// Pressure sensitive buttons
		m_upButtonPressure, m_rightButtonPressure, m_leftButtonPressure, m_downButtonPressure,
		m_crossButtonPressure, m_circleButtonPressure, m_squareButtonPressure, m_triangleButtonPressure,
		m_l1ButtonPressure, m_l2ButtonPressure, m_r1ButtonPressure, m_r2ButtonPressure};
	std::copy(std::begin(tempPressureButtons), std::end(tempPressureButtons), std::begin(m_buttonsPressure));

	// NOTE: Order MATTERS, these match enum defined in PadData.h
	wxSlider* tempm_analogSliders[4] = { m_leftAnalogXVal, m_leftAnalogYVal, m_rightAnalogXVal, m_rightAnalogYVal };
	std::copy(std::begin(tempm_analogSliders), std::end(tempm_analogSliders), std::begin(m_analogSliders));

	// NOTE: Order MATTERS, these match enum defined in PadData.h
	wxSpinCtrl* tempm_analogVals[4] = { m_leftAnalogXValPrecise, m_leftAnalogYValPrecise, m_rightAnalogXValPrecise, m_rightAnalogYValPrecise };
	std::copy(std::begin(tempm_analogVals), std::end(tempm_analogVals), std::begin(m_analogVals));

	// Setup event bindings
	for (int i = 0; i < m_buttonsLength; i++)
		(*m_buttons[i]).Bind(wxEVT_TOGGLEBUTTON, &VirtualPad::onButtonPress, this);
	for (int i = 0; i < m_buttonsPressureLength; i++)
		(*m_buttonsPressure[i]).Bind(wxEVT_SPINCTRL, &VirtualPad::onPressureChange, this);
	for (int i = 0; i < m_analogSlidersLength; i++)
		(*m_analogSliders[i]).Bind(wxEVT_SLIDER, &VirtualPad::onAnalogSliderChange, this);
	for (int i = 0; i < m_analogValsLength; i++)
		(*m_analogVals[i]).Bind(wxEVT_SPINCTRL, &VirtualPad::onAnalogValChange, this);

	// Finalize layout
	setProperties();
	doLayout();
}


void VirtualPad::setProperties()
{
	SetTitle(wxString::Format("Virtual Pad - Port %d", m_controllerPort + 1));
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
}

bool VirtualPad::Show(bool show)
{
	if (!wxFrame::Show(show))
		return false;
	if (show)
		g_RecordingInput.setVirtualPadReading(m_controllerPort, true);
	return true;
}

void VirtualPad::onClose(wxCloseEvent & event)
{
	g_RecordingInput.setVirtualPadReading(m_controllerPort, false);
	Hide();
}

void VirtualPad::onButtonPress(wxCommandEvent & event)
{
	wxToggleButton* pressedButton = (wxToggleButton*) event.GetEventObject();
	for (int i = 0; i < m_buttonsLength; i++)
	{
		if (pressedButton == m_buttons[i])
		{
			u8 pressure = 0;
			if (event.IsChecked())
				pressure = i < 12 ? m_buttonsPressure[i]->GetValue() : 255;
			g_RecordingInput.setButtonState(m_controllerPort, PadData_NormalButton(i), pressure);
			break;
		}
	}
}

void VirtualPad::onPressureChange(wxSpinEvent & event)
{
	wxSpinCtrl* updatedSpinner = (wxSpinCtrl*) event.GetEventObject();
	for (int i = 0; i < m_buttonsPressureLength; i++)
	{
		if (updatedSpinner == m_buttonsPressure[i])
		{
			u8 pressure = 0;
			if (event.IsChecked())
				pressure = m_buttonsPressure[i]->GetValue();
			g_RecordingInput.setButtonState(m_controllerPort, PadData_NormalButton(i), pressure);
			break;
		}
	}
}

void VirtualPad::onAnalogSliderChange(wxCommandEvent & event)
{
	wxSlider* movedSlider = (wxSlider*) event.GetEventObject();
	for (int i = 0; i < m_analogSlidersLength; i++)
	{
		if (movedSlider == m_analogSliders[i])
		{
			if (!(i & 1))
				m_analogVals[i]->SetValue(event.GetInt());
			else
				m_analogVals[i]->SetValue(event.GetInt() * -1);

			g_RecordingInput.updateAnalog(m_controllerPort, PadData_AnalogVector(i), event.GetInt() + 127);
			break;
		}
	}
}

void VirtualPad::onAnalogValChange(wxSpinEvent & event)
{
	wxSpinCtrl* updatedSpinner = (wxSpinCtrl*)event.GetEventObject();
	for (int i = 0; i < m_analogValsLength; i++)
	{
		if (updatedSpinner == m_analogVals[i])
		{
			m_analogVals[i]->SetValue(event.GetInt());
			g_RecordingInput.updateAnalog(m_controllerPort, PadData_AnalogVector(i), event.GetInt() + 127);
			break;
		}
	}
}

void VirtualPad::doLayout()
{
	wxBoxSizer* container = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* analogSticks = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* emptySpace6 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* rightAnalogContainer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* rightAnalog = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* rightAnalogYContainer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* rightAnalogY = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* rightAnalogButtonAndGUI = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* rightAnalogX = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* emptySpace5 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* leftAnalogContainer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* leftAnalog = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* leftAnalogYContainer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* leftAnalogY = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* leftAnalogButtonAndGUI = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* leftAnalogX = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* emptySpace4 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* faceButtonRow = new wxBoxSizer(wxHORIZONTAL);
	wxGridSizer* faceButtons = new wxGridSizer(0, 3, 0, 0);
	wxBoxSizer* cross = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* circle = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* square = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* triangle = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* middleOfController = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* emptySpace8 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* startAndSelect = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* emptySpace9 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* emptySpace7 = new wxBoxSizer(wxHORIZONTAL);
	wxGridSizer* dPad = new wxGridSizer(0, 3, 0, 0);
	wxBoxSizer* dPadDown = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* dPadRight = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* dPadLeft = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* dPadUp = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* shoulderButtons = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* emptySpace3 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* rightShoulderButtons = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* r1ButtonRow = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* r2ButtonRow = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* emptySpace2 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* leftShoulderButtons = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* l1ButtonRow = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* l2ButtonRow = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* emptySpace1 = new wxBoxSizer(wxVERTICAL);
	emptySpace1->Add(0, 0, 0, 0, 0);
	shoulderButtons->Add(emptySpace1, 2, wxEXPAND, 0);
	l2ButtonRow->Add(m_l2Button, 0, wxEXPAND, 0);
	l2ButtonRow->Add(m_l2ButtonPressure, 0, wxEXPAND, 0);
	leftShoulderButtons->Add(l2ButtonRow, 1, wxEXPAND, 0);
	l1ButtonRow->Add(m_l1Button, 0, wxEXPAND, 0);
	l1ButtonRow->Add(m_l1ButtonPressure, 0, wxEXPAND, 0);
	leftShoulderButtons->Add(l1ButtonRow, 1, wxEXPAND, 0);
	shoulderButtons->Add(leftShoulderButtons, 5, wxEXPAND, 0);
	emptySpace2->Add(0, 0, 0, 0, 0);
	shoulderButtons->Add(emptySpace2, 13, wxEXPAND, 0);
	r2ButtonRow->Add(m_r2Button, 0, wxEXPAND, 0);
	r2ButtonRow->Add(m_r2ButtonPressure, 0, wxEXPAND, 0);
	rightShoulderButtons->Add(r2ButtonRow, 1, wxEXPAND, 0);
	r1ButtonRow->Add(m_r1Button, 0, wxEXPAND, 0);
	r1ButtonRow->Add(m_r1ButtonPressure, 0, wxEXPAND, 0);
	rightShoulderButtons->Add(r1ButtonRow, 1, wxEXPAND, 0);
	shoulderButtons->Add(rightShoulderButtons, 5, wxEXPAND, 0);
	emptySpace3->Add(0, 0, 0, 0, 0);
	shoulderButtons->Add(emptySpace3, 2, wxEXPAND, 0);
	container->Add(shoulderButtons, 1, wxBOTTOM | wxEXPAND | wxTOP, 3);
	dPad->Add(0, 0, 0, 0, 0);
	dPadUp->Add(m_upButton, 0, wxEXPAND, 0);
	dPadUp->Add(m_upButtonPressure, 0, wxEXPAND, 0);
	dPad->Add(dPadUp, 1, wxEXPAND, 0);
	dPad->Add(0, 0, 0, 0, 0);
	dPadLeft->Add(m_leftButton, 0, wxEXPAND, 0);
	dPadLeft->Add(m_leftButtonPressure, 0, wxEXPAND, 0);
	dPad->Add(dPadLeft, 1, wxEXPAND, 0);
	dPad->Add(0, 0, 0, 0, 0);
	dPadRight->Add(m_rightButton, 0, wxEXPAND, 0);
	dPadRight->Add(m_rightButtonPressure, 0, wxEXPAND, 0);
	dPad->Add(dPadRight, 1, wxEXPAND, 0);
	dPad->Add(0, 0, 0, 0, 0);
	dPadDown->Add(m_downButton, 0, wxEXPAND, 0);
	dPadDown->Add(m_downButtonPressure, 0, wxEXPAND, 0);
	dPad->Add(dPadDown, 1, wxEXPAND, 0);
	dPad->Add(0, 0, 0, 0, 0);
	faceButtonRow->Add(dPad, 9, wxEXPAND | wxLEFT | wxRIGHT, 3);
	emptySpace7->Add(0, 0, 0, 0, 0);
	middleOfController->Add(emptySpace7, 1, wxEXPAND, 0);
	startAndSelect->Add(m_selectButton, 0, 0, 0);
	emptySpace9->Add(0, 0, 0, 0, 0);
	startAndSelect->Add(emptySpace9, 1, wxEXPAND, 0);
	startAndSelect->Add(m_startButton, 0, 0, 0);
	middleOfController->Add(startAndSelect, 1, wxEXPAND, 0);
	emptySpace8->Add(0, 0, 0, 0, 0);
	middleOfController->Add(emptySpace8, 1, wxEXPAND, 0);
	faceButtonRow->Add(middleOfController, 8, wxEXPAND | wxLEFT | wxRIGHT, 3);
	faceButtons->Add(0, 0, 0, 0, 0);
	triangle->Add(m_triangleButton, 0, wxEXPAND, 0);
	triangle->Add(m_triangleButtonPressure, 0, wxEXPAND, 0);
	faceButtons->Add(triangle, 1, wxEXPAND, 0);
	faceButtons->Add(0, 0, 0, 0, 0);
	square->Add(m_squareButton, 0, wxEXPAND, 0);
	square->Add(m_squareButtonPressure, 0, wxEXPAND, 0);
	faceButtons->Add(square, 1, wxEXPAND, 0);
	faceButtons->Add(0, 0, 0, 0, 0);
	circle->Add(m_circleButton, 0, wxEXPAND, 0);
	circle->Add(m_circleButtonPressure, 0, wxEXPAND, 0);
	faceButtons->Add(circle, 1, wxEXPAND, 0);
	faceButtons->Add(0, 0, 0, 0, 0);
	cross->Add(m_crossButton, 0, wxEXPAND, 0);
	cross->Add(m_crossButtonPressure, 0, wxEXPAND, 0);
	faceButtons->Add(cross, 1, wxEXPAND, 0);
	faceButtons->Add(0, 0, 0, 0, 0);
	faceButtonRow->Add(faceButtons, 9, wxEXPAND | wxLEFT | wxRIGHT, 3);
	container->Add(faceButtonRow, 4, wxBOTTOM | wxEXPAND | wxTOP, 3);
	emptySpace4->Add(0, 0, 0, 0, 0);
	analogSticks->Add(emptySpace4, 6, wxEXPAND, 0);
	leftAnalogX->Add(m_leftAnalogXVal, 1, wxALL | wxEXPAND, 0);
	leftAnalogX->Add(m_leftAnalogXValPrecise, 0, wxEXPAND, 0);
	leftAnalog->Add(leftAnalogX, 1, wxEXPAND, 0);
	leftAnalogButtonAndGUI->Add(0, 0, 0, 0, 0);
	leftAnalogButtonAndGUI->Add(m_l3Button, 0, wxALIGN_CENTER, 0);
	leftAnalogYContainer->Add(leftAnalogButtonAndGUI, 1, wxEXPAND, 0);
	leftAnalogY->Add(m_leftAnalogYVal, 1, wxALIGN_RIGHT, 0);
	leftAnalogY->Add(m_leftAnalogYValPrecise, 0, wxALIGN_RIGHT, 0);
	leftAnalogYContainer->Add(leftAnalogY, 1, wxEXPAND, 0);
	leftAnalog->Add(leftAnalogYContainer, 5, wxEXPAND, 0);
	leftAnalogContainer->Add(leftAnalog, 1, wxEXPAND, 0);
	analogSticks->Add(leftAnalogContainer, 6, wxEXPAND, 0);
	emptySpace5->Add(0, 0, 0, 0, 0);
	analogSticks->Add(emptySpace5, 3, wxEXPAND, 0);
	rightAnalogX->Add(m_rightAnalogXVal, 1, wxEXPAND, 0);
	rightAnalogX->Add(m_rightAnalogXValPrecise, 0, wxEXPAND, 0);
	rightAnalog->Add(rightAnalogX, 1, wxEXPAND, 0);
	rightAnalogButtonAndGUI->Add(0, 0, 0, 0, 0);
	rightAnalogButtonAndGUI->Add(m_r3Button, 0, wxALIGN_CENTER, 0);
	rightAnalogYContainer->Add(rightAnalogButtonAndGUI, 1, wxEXPAND, 0);
	rightAnalogY->Add(m_rightAnalogYVal, 1, wxALIGN_RIGHT, 0);
	rightAnalogY->Add(m_rightAnalogYValPrecise, 0, wxALIGN_RIGHT | wxEXPAND, 0);
	rightAnalogYContainer->Add(rightAnalogY, 1, wxEXPAND, 0);
	rightAnalog->Add(rightAnalogYContainer, 5, wxEXPAND, 0);
	rightAnalogContainer->Add(rightAnalog, 1, wxEXPAND, 0);
	analogSticks->Add(rightAnalogContainer, 6, wxEXPAND, 0);
	emptySpace6->Add(0, 0, 0, 0, 0);
	analogSticks->Add(emptySpace6, 6, wxEXPAND, 0);
	container->Add(analogSticks, 3, wxBOTTOM | wxEXPAND | wxTOP, 3);
	SetSizer(container);
	Layout();
}
#endif

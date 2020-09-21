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

#include <wx/spinctrl.h>

#include "Recording/VirtualPad/VirtualPadResources.h"
#include "Recording/PadData.h"

void ControllerNormalButton::UpdateGuiElement(std::queue<VirtualPadElement*> *renderQueue, bool &clearScreenRequired)
{
    ControllerNormalButton &button = *this;
    if (button.widgetUpdateRequired)
	{
        button.pressedBox->SetValue(button.pressed);
	}

    if (button.pressed)
	{
        renderQueue->push(this);
    } 
	else if (button.currentlyRendered) 
	{
        button.currentlyRendered = false;
        clearScreenRequired = true;
	}
}

void ControllerPressureButton::UpdateGuiElement(std::queue<VirtualPadElement *> *renderQueue, bool &clearScreenRequired)
{
    ControllerPressureButton &button = *this;
    if (button.widgetUpdateRequired) 
	{
        button.pressureSpinner->SetValue(button.pressure);
        clearScreenRequired = true;
    }

    if (button.pressed) 
	{
        renderQueue->push(this);
    } 
	else if (button.currentlyRendered) 
	{
        button.currentlyRendered = false;
        clearScreenRequired = true;
    }
}

void AnalogStick::UpdateGuiElement(std::queue<VirtualPadElement *> *renderQueue, bool &clearScreenRequired)
{
    AnalogStick &analogStick = *this;
    // Update the GUI elements that need updating
    // If either vector has changed, we need to redraw the graphics
    if (analogStick.xVector.widgetUpdateRequired) 
	{
        analogStick.xVector.slider->SetValue(analogStick.xVector.val);
        analogStick.xVector.spinner->SetValue(analogStick.xVector.val);
    }
    if (analogStick.yVector.widgetUpdateRequired) 
	{
        analogStick.yVector.slider->SetValue(analogStick.yVector.val);
		analogStick.yVector.spinner->SetValue(analogStick.yVector.val);
    }
    if (!(analogStick.xVector.val == PadData::ANALOG_VECTOR_NEUTRAL && analogStick.yVector.val == PadData::ANALOG_VECTOR_NEUTRAL))
	{
        renderQueue->push(this);
	} 
	else if (analogStick.currentlyRendered) {
        analogStick.currentlyRendered = false;
        clearScreenRequired = true;
    }
}

void ControllerNormalButton::Render(wxDC &dc)
{
    ControllerNormalButton &button = *this;
    ImageFile &img = button.icon;
    dc.DrawBitmap(img.image, img.coords, true);
    button.currentlyRendered = true;
}

void ControllerPressureButton::Render(wxDC &dc)
{
    ControllerPressureButton &button = *this;
    ImageFile &img = button.icon;
    dc.DrawBitmap(img.image, img.coords, true);
    button.currentlyRendered = true;
}

void AnalogStick::Render(wxDC &dc)
{
    AnalogStick &analogStick = *this;
    // Render graphic
    AnalogPosition analogPos = analogStick.positionGraphic;
    // Determine new end coordinates
    int newXCoord = analogPos.centerCoords.x + ((analogStick.xVector.val - 127) / 127.0) * analogPos.radius;
    int newYCoord = analogPos.centerCoords.y + ((analogStick.yVector.val - 127) / 127.0) * analogPos.radius;
    // We want to ensure the line segment length is capped at the defined radius
    float lengthOfLine = sqrt(pow(newXCoord - analogPos.centerCoords.x, 2) + pow(newYCoord - analogPos.centerCoords.y, 2));
    if (lengthOfLine > analogPos.radius) {
        newXCoord = ((1 - analogPos.radius / lengthOfLine) * analogPos.centerCoords.x) + analogPos.radius / lengthOfLine * newXCoord;
        newYCoord = ((1 - analogPos.radius / lengthOfLine) * analogPos.centerCoords.y) + analogPos.radius / lengthOfLine * newYCoord;
    }
    // Set the new end coordinate
    analogPos.endCoords = wxPoint(newXCoord, newYCoord);
    // Draw line and tip
    dc.SetPen(wxPen(*wxBLUE, analogPos.lineThickness));
    dc.DrawLine(analogPos.centerCoords, analogPos.endCoords);
    dc.DrawCircle(analogPos.endCoords, wxCoord(analogPos.lineThickness));
    dc.SetPen(wxNullPen);
    analogStick.currentlyRendered = true;
}

bool ControllerButton::UpdateButtonData(bool &padDataVal, bool ignoreRealController, bool readOnly)
{
    ControllerButton &button = *this;
    if (!ignoreRealController) {
        // If controller is being bypassed and controller's state has changed
        bool bypassedWithChangedState = button.isControllerPressBypassed && padDataVal != button.prevPressedVal;
        if (bypassedWithChangedState) {
            button.prevPressedVal = padDataVal;
            button.isControllerPressBypassed = false;
        }
        // If we aren't bypassing the controller OR the previous condition was met
        if (bypassedWithChangedState || !button.isControllerPressBypassed) {
            button.widgetUpdateRequired = button.pressed != padDataVal;
            button.pressed = padDataVal;
            return false;
        }
    }
    button.prevPressedVal = padDataVal;
    padDataVal = button.pressed;
    return button.prevPressedVal != button.pressed;
}

bool ControllerNormalButton::UpdateData(bool &padDataVal, bool ignoreRealController, bool readOnly)
{
    return this->UpdateButtonData(padDataVal, ignoreRealController, readOnly);
}

bool ControllerPressureButton::UpdateData(bool &padDataVal, bool ignoreRealController, bool readOnly)
{
    return this->UpdateButtonData(padDataVal, ignoreRealController, readOnly);
}

bool ControllerPressureButton::UpdateData(u8 &padDataVal, bool ignoreRealController, bool readOnly)
{
    ControllerPressureButton &button = *this;
    if (!ignoreRealController) {
        bool bypassedWithChangedState = button.isControllerPressureBypassed && padDataVal != button.prevPressureVal;
        if (bypassedWithChangedState) {
            button.prevPressureVal = padDataVal;
            button.isControllerPressureBypassed = false;
        }
        if (bypassedWithChangedState || !button.isControllerPressureBypassed) {
            button.widgetUpdateRequired = button.pressure != padDataVal;
            button.pressure = padDataVal;
            return false;
        }
    }
    button.prevPressureVal = padDataVal;
    padDataVal = button.pressure;
    return button.prevPressedVal != button.pressure;
}

bool AnalogVector::UpdateData(u8 &padDataVal, bool ignoreRealController, bool readOnly)
{
    AnalogVector &vector = *this;
    if (!ignoreRealController) {
        bool bypassedWithChangedState = vector.isControllerBypassed && padDataVal != vector.prevVal;
        if (bypassedWithChangedState) {
            vector.prevVal = padDataVal;
            vector.isControllerBypassed = false;
        }
        if (bypassedWithChangedState || !vector.isControllerBypassed) {
            vector.widgetUpdateRequired = vector.val != padDataVal;
            vector.val = padDataVal;
            return false;
        }
    }
    vector.prevVal = padDataVal;
    padDataVal = vector.val;
    return vector.prevVal != vector.val;
}
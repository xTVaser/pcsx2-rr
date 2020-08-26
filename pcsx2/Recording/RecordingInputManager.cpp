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

#include "RecordingInputManager.h"
#include "InputRecording.h"


#ifndef DISABLE_RECORDING
RecordingInputManager g_RecordingInput;

RecordingInputManager::RecordingInputManager()
{
	for (u8 i = 0; i < 2; i++)
	{
		virtualPad[i] = false;
	}
}

void RecordingInputManager::ControllerInterrupt(const u8 data, const u8 port, const u16 buf_count, u8(&buf)[512])
{
	if (virtualPad[port])
	{
		int buf_index = buf_count - 3;
		// first two bytes have nothing of interest in the buffer
		// already handled by InputRecording.cpp
		if (buf_count < 3)
		{
			return;
		}

		// Normal keys
		// We want to perform an OR, but, since 255 means that no button is pressed and 0 that every button is pressed (and by De Morgan's Laws), we execute an AND.
		if (buf_count <= 4)
		{
			buf[buf_count] = buf[buf_count] & pad.buf[port][buf_index];
		}
		// Analog keys (! overrides !)
		else if ((buf_count > 4 && buf_count <= 6) && pad.buf[port][buf_index] != 127)
		{
			buf[buf_count] = pad.buf[port][buf_index];
		}
		// Pressure sensitivity bytes
		else if (buf_count > 6)
		{
			buf[buf_count] = pad.buf[port][buf_index];
		}

		// Updating movie file
		g_InputRecording.controllerInterrupt(data, port, buf_count, buf);
	}
}

void RecordingInputManager::SetButtonState(int port, PadData_NormalButton button, int pressure)
{
	std::vector<int> buttons = pad.GetNormalButtons(port);
	buttons[button] = pressure;
	pad.SetNormalButtons(port, buttons);
}

void RecordingInputManager::UpdateAnalog(int port, PadData_AnalogVector vector, int value)
{
	std::vector<int> vectors = pad.GetAnalogVectors(port);
	vectors[vector] = value;
	pad.SetAnalogVectors(port, vectors);
}

void RecordingInputManager::SetVirtualPadReading(int port, bool read)
{
	virtualPad[port] = read;
}
#endif

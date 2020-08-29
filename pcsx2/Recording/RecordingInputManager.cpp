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
		m_virtualPad[i] = false;
}

void RecordingInputManager::controllerInterrupt(const u8 data, const u8 port, const u16 bufCount, u8(&buf)[512])
{
	if (m_virtualPad[port])
	{
		// first two bytes have nothing of interest in the buffer
		// already handled by InputRecording.cpp
		if (bufCount < 3)
			return;
		
		// Normal keys
		// We want to perform an OR, but, since 255 means that no button is pressed and 0 that every button is pressed (and by De Morgan's Laws), we execute an AND.
		if (bufCount <= 4)
			buf[bufCount] = buf[bufCount] & m_pad.m_buf[port][bufCount - 3];
		else
		{
			int bufIndex = bufCount - 3;
			// Analog keys (! overrides !)
			if ((bufCount > 4 && bufCount <= 6) && m_pad.m_buf[port][bufIndex] != 127)
				buf[bufCount] = m_pad.m_buf[port][bufIndex];
			// Pressure sensitivity bytes
			else if (bufCount > 6)
				buf[bufCount] = m_pad.m_buf[port][bufIndex];
		}

		// Updating movie file
		g_InputRecording.controllerInterrupt(data, port, bufCount, buf);
	}
}

void RecordingInputManager::setButtonState(int port, PadData_NormalButton button, int pressure)
{
	std::vector<int> buttons = m_pad.getNormalButtons(port);
	buttons[button] = pressure;
	m_pad.setNormalButtons(port, buttons);
}

void RecordingInputManager::updateAnalog(int port, PadData_AnalogVector vector, int value)
{
	std::vector<int> vectors = m_pad.getAnalogVectors(port);
	vectors[vector] = value;
	m_pad.setAnalogVectors(port, vectors);
}

void RecordingInputManager::setVirtualPadReading(int port, bool read)
{
	m_virtualPad[port] = read;
}
#endif

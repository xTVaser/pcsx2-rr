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

#include "PadData.h"


#ifndef DISABLE_RECORDING
class RecordingInputManager
{
public:
	RecordingInputManager();

	void controllerInterrupt(const u8 data, const u8 port, const u16 bufCount, u8 (&buf)[512]);
	// Handles normal keys
	void setButtonState(int port, PadData_NormalButton button, int pressure);
	// Handles analog sticks
	void updateAnalog(int port, PadData_AnalogVector vector, int value);
	void setVirtualPadReading(int port, bool read);

protected:
	PadData m_pad;
	bool m_virtualPad[2];
};

extern RecordingInputManager g_RecordingInput;
#endif

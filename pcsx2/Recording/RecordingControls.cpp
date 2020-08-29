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

#include "App.h"
#include "Counters.h"
#include "Common.h"
#include "GSFrame.h"
#include "MemoryTypes.h"

#include "RecordingControls.h"


#ifndef DISABLE_RECORDING
RecordingControls g_RecordingControls;

//-----------------------------------------------
// Current recording status, returns true if:
// - Recording is Paused
// - GSFrame CoreThread is both running AND paused
//-----------------------------------------------
bool RecordingControls::isRecordingPaused()
{
	return m_PauseState && CoreThread.IsOpen() && CoreThread.IsPaused();
}

//-----------------------------------------------
// Called after inputs are recorded for that frame, places lock on input recording, effectively releasing resources and resuming CoreThread.
//-----------------------------------------------
void RecordingControls::resumeCoreThreadIfStarted()
{
	if (m_Start && CoreThread.IsOpen() && CoreThread.IsPaused())
	{
		CoreThread.Resume();
		m_Start = false;
		m_PauseState = false;
	}
}

//-----------------------------------------------
// Called at VSYNC End / VRender Begin, updates everything recording related for the next frame,
// toggles RecordingControl flags back to enable input recording for the next frame.
//-----------------------------------------------
void RecordingControls::handleframeAdvanceAndStop()
{
	if (m_frameAdvance)
	{
		if (m_stopFrameCount < g_FrameCount)
		{
			m_frameAdvance = false;
			m_Stop = true;
			m_stopFrameCount = g_FrameCount;

			// We force the frame counter in the title bar to change
			wxString oldTitle = wxGetApp().GetGsFrame().GetTitle();
			wxString title = g_Conf->Templates.RecordingTemplate;
			wxString frameCount = wxString::Format("%d", g_FrameCount);

			title.Replace(L"${frame}", frameCount);
			int frameIndex = title.find(wxString::Format(L"%d", g_FrameCount));
			frameIndex += frameCount.length();

			title.replace(frameIndex, oldTitle.length() - frameIndex, oldTitle.c_str().AsChar() + frameIndex);

			wxGetApp().GetGsFrame().SetTitle(title);
		}
	}
	if (m_Stop && CoreThread.IsOpen() && CoreThread.IsRunning())
	{
		m_PauseState = true;
		CoreThread.PauseSelf();
	}
}

bool RecordingControls::getStopFlag() noexcept
{
	return (m_Stop || m_frameAdvance);
}

void RecordingControls::frameAdvance() noexcept
{
	m_stopFrameCount = g_FrameCount;
	m_frameAdvance = true;
	m_Stop = false;
	m_Start = true;
}

void RecordingControls::togglePause() noexcept
{
	m_Stop = !m_Stop;
	if (m_Stop == false)
		m_Start = true;
}

void RecordingControls::pause() noexcept
{
	m_Stop = true;
}

void RecordingControls::resume() noexcept
{
	m_Stop = false;
	m_Start = true;
}
#endif

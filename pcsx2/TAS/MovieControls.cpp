#include "PrecompiledHeader.h"

#include "GSFrame.h"

#include "MemoryTypes.h"
#include "App.h"	// use "CoreThread"
#include "Counters.h"	// use"g_FrameCount"

#include "MovieControls.h"


MovieControls g_MovieControls;


//-----------------------------------------------
// The game is running with CoreThread, and it will not accept processing of GSFrame (wxFrame) while it is running
// (It seems that two places are accepted for key input within CoreThread and GSFrame, too)
// While CoreThread is stopped, the input of wxFrame works like a mechanism
// �Q�[����CoreThread�œ����Ă���A�����Ă���Ԃ�GSFrame(wxFrame)�̏�����󂯕t���Ȃ�
// (�L�[���͂̎󂯕t����CoreThread���GSFrame���2�ӏ����ݒ肳��Ă���ۂ�)
// CoreThread����~���Ă���Ԃ�wxFrame�̓��͂������d�g�݂��ۂ�
//-----------------------------------------------

bool MovieControls::isStop()
{
	return (fPauseState && CoreThread.IsOpen() && CoreThread.IsPaused());
}
//-----------------------------------------------
// Counters(CoreThread)��̒�~����p (For stop judgment in)
//-----------------------------------------------
void MovieControls::StartCheck()
{
	if (fStart && CoreThread.IsOpen() && CoreThread.IsPaused()) {
		CoreThread.Resume();
		fStart = false;
		fPauseState = false;
	}
}

void MovieControls::StopCheck()
{
	if (fFrameAdvance) {
		if (stopFrameCount < g_FrameCount) {
			fFrameAdvance = false;
			fStop = true;
			stopFrameCount = g_FrameCount;

			// We force the frame counter in the title bar to change
			wxString oldTitle = wxGetApp().GetGsFrame().GetTitle();
			wxString title = g_Conf->Templates.TASTemplate;
			wxString frameCount = wxString::Format("%d", g_FrameCount);

			title.Replace(L"${frame}", frameCount);	//--TAS--//
			int frameIndex = title.find(wxString::Format(L"%d", g_FrameCount));
			frameIndex += frameCount.length();

			title.replace(frameIndex, oldTitle.length() - frameIndex, oldTitle.c_str().AsChar() + frameIndex);

			wxGetApp().GetGsFrame().SetTitle(title);
		}
	}
	if (fStop && CoreThread.IsOpen() && CoreThread.IsRunning())
	{
		fPauseState = true;
		CoreThread.PauseSelf();	// I can not stop unless it is self
								// self����Ȃ��Ǝ~�܂�Ȃ�
	}
}



//----------------------------------
// shortcut key
//----------------------------------
void MovieControls::FrameAdvance()
{
	stopFrameCount = g_FrameCount;
	fFrameAdvance = true;
	fStop = false;
	fStart = true;
}
void MovieControls::TogglePause()
{
	fStop = !fStop;
	if (fStop == false) {
		fStart = true;
	}
}
void MovieControls::Pause()
{
	fStop = true;
	fFrameAdvance = true;
}
void MovieControls::UnPause()
{
	fStop = false;
	fStart = true;
	fFrameAdvance = true;
}


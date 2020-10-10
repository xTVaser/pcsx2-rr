#include "PrecompiledHeader.h"
#include "GuiConfig.h"

ConsoleLogOptions::ConsoleLogOptions()
	: DisplayPosition{100, 100}
	, DisplaySize{680, 560}
	, Theme("Default")
{
	Visible = true;
	AutoDock = true;
	FontSize = 8;

	conf = new wxConfig("ConsoleLogOptions");
	conf->Create();
}

bool ConsoleLogOptions::Save()
{
	conf->Write(L"Theme", Theme);
	conf->Write(L"FontSize", FontSize);
	conf->Write(L"IsVisible", Visible);
	conf->Write(L"Autodock", AutoDock);
	conf->Write(L"DisplaySize", DisplaySize);
	conf->Write(L"DisplayPosition", DisplayPosition);
}

void ConsoleLogOptions::Load()
{
	conf->Read(L"Theme", Theme);
	conf->Read(L"FontSize", FontSize);
	conf->Read(L"IsVisible", Visible);
	conf->Read(L"Autodock", AutoDock);
	conf->ReadObject(L"DisplaySize", DisplaySize);
	conf->ReadObject(L"DisplayPosition", DisplayPosition);
}

GSWindowOptions::GSWindowOptions()
{
	CloseOnEsc = true;
	DefaultToFullscreen = false;
	AlwaysHideMouse = false;
	DisableResizeBorders = false;
	DisableScreenSaver = true;

	AspectRatio = AspectRatio_4_3;
	FMVAspectRatioSwitch = FMV_AspectRatio_Switch_Off;
	Zoom = 100;
	StretchY = 100;
	OffsetX = 0;
	OffsetY = 0;

	WindowSize = wxSize(640, 480);
	WindowPos = wxDefaultPosition;
	IsMaximized = false;
	IsFullscreen = false;
	EnableVsyncWindowFlag = false;

	IsToggleFullscreenOnDoubleClick = true;

	conf = new wxConfig("GSWindowOptions");
	conf->Create();
}

void GSWindowOptions::Save()
{
	conf->Write("Zoom", Zoom);
	conf->Write("OffsetX", OffsetX);
	conf->Write("OffsetY", OffsetY);
	conf->Write("StretchY", StretchY);
	conf->Write("WindowPos", WindowPos);
	conf->Write("WindowSize", WindowSize);
	conf->Write("CloseOnEsc", CloseOnEsc);
	conf->Write("AspectRatio", AspectRatio);
	conf->Write("IsMaximized", IsMaximized);
	conf->Write("IsFullscreen", IsFullscreen);
	conf->Write("AlwaysHideMouse", AlwaysHideMouse);
	conf->Write("DisableScreenSaver", DisableScreenSaver);
	conf->Write("DefaultToFullScreen", DefaultToFullscreen);
	conf->Write("DisableResizeBorders", DisableResizeBorders);
	conf->Write("FMVAspectRatioSwitch", FMVAspectRatioSwitch);
	conf->Write("EnableVsyncWindowFlag", EnableVsyncWindowFlag);
	conf->Write("IsToggleFullscreenOnDoubleClick", IsToggleFullscreenOnDoubleClick);
}

void GSWindowOptions::Load()
{
	conf->ReadObject("Zoom", Zoom);
	conf->ReadObject("OffsetX", OffsetX);
	conf->ReadObject("OffsetY", OffsetY);
	conf->ReadObject("StretchY", StretchY);
	conf->ReadObject("WindowPos", WindowPos);
	conf->ReadObject("WindowSize", WindowSize);
	conf->Read("CloseOnEsc", CloseOnEsc);
	conf->Read("AspectRatio", AspectRatio);
	conf->Read("IsMaximized", IsMaximized);
	conf->Read("IsFullscreen", IsFullscreen);
	conf->Read("AlwaysHideMouse", AlwaysHideMouse);
	conf->Read("DisableScreenSaver", DisableScreenSaver);
	conf->Read("DefaultToFullScreen", DefaultToFullscreen);
	conf->Read("DisableResizeBorders", DisableResizeBorders);
	conf->Read("FMVAspectRatioSwitch", FMVAspectRatioSwitch);
	conf->Read("EnableVsyncWindowFlag", EnableVsyncWindowFlag);
	conf->Read("IsToggleFullscreenOnDoubleClick", IsToggleFullscreenOnDoubleClick);
}

void GSWindowOptions::SanityCheck()
{
	// Ensure Conformation of various options...

	WindowSize.x = std::max(WindowSize.x, 8);
	WindowSize.x = std::min(WindowSize.x, wxGetDisplayArea().GetWidth() - 16);

	WindowSize.y = std::max(WindowSize.y, 8);
	WindowSize.y = std::min(WindowSize.y, wxGetDisplayArea().GetHeight() - 48);

	// Make sure the upper left corner of the window is visible enought o grab and
	// move into view:
	if (!wxGetDisplayArea().Contains(wxRect(WindowPos, wxSize(48, 48))))
		WindowPos = wxDefaultPosition;

	if ((uint)AspectRatio >= (uint)AspectRatio_MaxCount)
		AspectRatio = AspectRatio_4_3;
}

void GuiConfig::Load()
{
	console.Load();
	gsWindow.Load();
}


void GuiConfig::Save()
{
	console.Save();
	console.Load();
}
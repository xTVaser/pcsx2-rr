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
}

bool ConsoleLogOptions::Save(wxConfig* conf)
{
	conf->Write("Theme", Theme);
	conf->Write("FontSize", FontSize);
	conf->Write("IsVisible", Visible);
	conf->Write("Autodock", AutoDock);
	conf->Write("DisplaySizeX", DisplaySize.x);
	conf->Write("DisplaySizeY", DisplaySize.y);
	conf->Write("DisplayPositionX", DisplayPosition.x);
	conf->Write("DisplayPositionY", DisplayPosition.y);
	return true;
}

void ConsoleLogOptions::Load(wxConfig* conf)
{
	conf->Read("Theme", Theme);
	conf->Read("FontSize", FontSize);
	conf->Read("IsVisible", Visible);
	conf->Read("Autodock", AutoDock);
	conf->ReadObject("DisplaySizeX", DisplaySize.x);
	conf->ReadObject("DisplaySizeY", DisplaySize.y);
	conf->ReadObject("DisplayPositionX", DisplayPosition.x);
	conf->ReadObject("DisplayPositionY", DisplayPosition.y);
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
}

void GSWindowOptions::Save(wxConfig* conf)
{
	conf->Write("Zoom", Zoom);
	conf->Write("OffsetX", OffsetX);
	conf->Write("OffsetY", OffsetY);
	conf->Write("StretchY", StretchY);
	conf->Write("WindowPosX", WindowPos.x);
	conf->Write("WindowPosY", WindowPos.y);
	conf->Write("WindowSizeX", WindowSize.x);
	conf->Write("WindowSizeY", WindowSize.y);
	conf->Write("CloseOnEsc", CloseOnEsc);
	conf->Write("AspectRatio", (int)AspectRatio);
	conf->Write("IsMaximized", IsMaximized);
	conf->Write("IsFullscreen", IsFullscreen);
	conf->Write("AlwaysHideMouse", AlwaysHideMouse);
	conf->Write("DisableScreenSaver", DisableScreenSaver);
	conf->Write("DefaultToFullScreen", DefaultToFullscreen);
	conf->Write("DisableResizeBorders", DisableResizeBorders);
	conf->Write("FMVAspectRatioSwitch", (int)FMVAspectRatioSwitch);
	conf->Write("EnableVsyncWindowFlag", EnableVsyncWindowFlag);
	conf->Write("IsToggleFullscreenOnDoubleClick", IsToggleFullscreenOnDoubleClick);
}

void GSWindowOptions::Load(wxConfig* conf)
{
	conf->Read("Zoom", Zoom);
	conf->Read("OffsetX", OffsetX);
	conf->Read("OffsetY", OffsetY);
	conf->Read("StretchY", StretchY);
	conf->ReadObject("WindowPosX", WindowPos.x);
	conf->ReadObject("WindowPosY", WindowPos.y);
	conf->ReadObject("WindowSizeX", WindowSize.x);
	conf->ReadObject("WindowSizeY", WindowSize.y);
	conf->Read("CloseOnEsc", CloseOnEsc);
	conf->Read("AspectRatio", (int)AspectRatio);
	conf->Read("IsMaximized", IsMaximized);
	conf->Read("IsFullscreen", IsFullscreen);
	conf->Read("AlwaysHideMouse", AlwaysHideMouse);
	conf->Read("DisableScreenSaver", DisableScreenSaver);
	conf->Read("DefaultToFullScreen", DefaultToFullscreen);
	conf->Read("DisableResizeBorders", DisableResizeBorders);
	conf->Read("FMVAspectRatioSwitch", (int)FMVAspectRatioSwitch);
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

GuiConfig::GuiConfig()
	: MainGuiPosition( wxDefaultPosition )
	, SysSettingsTabName( "Cpu" )
	, McdSettingsTabName( "none" )
	, ComponentsTabName( "Plugins" )
	, AppSettingsTabName( "none" )
	, GameDatabaseTabName( "none" )
{
    LanguageId			= wxLANGUAGE_DEFAULT;
    LanguageCode		= "default";
    conf = std::make_unique<wxConfig>("PCSX2");
}

void GuiConfig::Init()
{

	conf = std::make_unique<wxConfig>("PCSX2");
	
	#ifdef __linux__
	    std::string programFullPath = wxStandardPaths::Get().GetExecutablePath().ToStdString();
	    std::string programDir(Path::Combine(programFullPath, "json/PCSX2_ui.ini"));	
		conf->SetPath(programDir);
	#endif

	isInit = true;

	Console.WriteLn(L"Path: " +  conf->GetPath());
}

void GuiConfig::Load()
{	
	if (!isInit)
	{
		Init();
	}

	console.Load(conf.get());	
	gsWindow.Load(conf.get());
}


void GuiConfig::Save()
{

	if (!isInit)
	{
		Init();
	}

	console.Save(conf.get());
	gsWindow.Save(conf.get());

    conf->Write("MainGuiPositionX", MainGuiPosition.x);
    conf->Write("MainGuiPositionY", MainGuiPosition.y);
    conf->Write("SysSettingsTabName", SysSettingsTabName);
	conf->Write("McdSettingsTabName", McdSettingsTabName);
	conf->Write("ComponentsTabName", ComponentsTabName);
	conf->Write("AppSettingsTabName", AppSettingsTabName);
	conf->Write("GameDatabaseTabName", GameDatabaseTabName);
    conf->Write("LanguageId", (int)LanguageId);
    conf->Write("LanguageCode", LanguageCode);
	conf->Flush();
}

GuiConfig::~GuiConfig()
{
	conf.release();
}
#pragma once
#include "AppForwardDefs.h"
#include "Utilities/wxGuiTools.h"
#include "Utilities/PathUtils.h"

#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/fileconf.h>
#include <wx/apptrait.h>
#include <iostream>
#include <iomanip>
#include <memory>

enum AspectRatioType
{
	AspectRatio_Stretch,
	AspectRatio_4_3,
	AspectRatio_16_9,
	AspectRatio_MaxCount
};

enum FMVAspectRatioSwitchType
{
	FMV_AspectRatio_Switch_Off,
	FMV_AspectRatio_Switch_4_3,
	FMV_AspectRatio_Switch_16_9,
	FMV_AspectRatio_Switch_MaxCount
};

	// ------------------------------------------------------------------------
struct ConsoleLogOptions
{
	bool Visible;
	// if true, DisplayPos is ignored and the console is automatically docked to the main window.
	bool AutoDock;
	// Display position used if AutoDock is false (ignored otherwise)
	wxPoint DisplayPosition;
	wxSize DisplaySize;

	// Size of the font in points.
	int FontSize;

	// Color theme by name!
	wxString Theme;

	ConsoleLogOptions();
	bool Save(wxConfig* conf);
	void Load(wxConfig* conf);
};

	// ------------------------------------------------------------------------
// The GS window receives much love from the land of Options and Settings.
//
struct GSWindowOptions
{
	// Closes the GS/Video port on escape (good for fullscreen activity)
	bool CloseOnEsc;

	bool DefaultToFullscreen;
	bool AlwaysHideMouse;
	bool DisableResizeBorders;
	bool DisableScreenSaver;

	AspectRatioType AspectRatio;
	FMVAspectRatioSwitchType FMVAspectRatioSwitch;
	float Zoom;
	float StretchY;
	float OffsetX;
	float OffsetY;

	wxSize WindowSize;
	wxPoint WindowPos;
	bool IsMaximized;
	bool IsFullscreen;
	bool EnableVsyncWindowFlag;

	bool IsToggleFullscreenOnDoubleClick;

	GSWindowOptions();

	void Save(wxConfig* conf);
	void Load(wxConfig* conf);
	void SanityCheck();
};

class GuiConfig
{	

private:
	// The Configurator
    std::unique_ptr<wxConfig> conf;
	bool isInit = false;

public:	
	ConsoleLogOptions console;
	GSWindowOptions gsWindow;	
	wxPoint		MainGuiPosition;

	// Because remembering the last used tab on the settings panel is cool (tab is remembered
	// by it's UTF/ASCII name).
	wxString	SysSettingsTabName;
	wxString	McdSettingsTabName;
	wxString	ComponentsTabName;
	wxString	AppSettingsTabName;
	wxString	GameDatabaseTabName;

	// Currently selected language ID -- wxWidgets version-specific identifier.  This is one side of
	// a two-part configuration that also includes LanguageCode.
	wxLanguage	LanguageId;

	// Current language in use (correlates to the universal language codes, such as "en_US", "de_DE", etc).
	// This code is not always unique, which is why we use the language ID also.
	wxString	LanguageCode;

	GuiConfig();
	void Init();
	void Load();
	void Save();
	~GuiConfig();
};

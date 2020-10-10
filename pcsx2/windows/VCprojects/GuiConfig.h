#pragma once
#include "AppForwardDefs.h"
#include "PathDefs.h"
#include "App.h"

#include <wx/config.h>
#include <wx/stdpaths.h>
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

	// The Configurator
	wxConfig* conf;

	// Size of the font in points.
	int FontSize;

	// Color theme by name!
	wxString Theme;

	ConsoleLogOptions();
	bool Save();
	void Load();
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
	Fixed100 Zoom;
	Fixed100 StretchY;
	Fixed100 OffsetX;
	Fixed100 OffsetY;

	wxConfig* conf;

	wxSize WindowSize;
	wxPoint WindowPos;
	bool IsMaximized;
	bool IsFullscreen;
	bool EnableVsyncWindowFlag;

	bool IsToggleFullscreenOnDoubleClick;

	GSWindowOptions();

	void Save();
	void Load();
	void SanityCheck();
};

struct GuiConfig
{	
public:	
	ConsoleLogOptions console;
	GSWindowOptions gsWindow;	
	void Load();	
	void Save();
};

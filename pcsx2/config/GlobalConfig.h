#pragma once

#include "EmulatorConfig.h"
#include "GuiConfig.h"

class GlobalConfig
{
public:
	GuiConfig* gui;
	Pcsx2Config* emulator;

	GlobalConfig();
	~GlobalConfig();
};

extern std::unique_ptr<GlobalConfig> g_Conf;

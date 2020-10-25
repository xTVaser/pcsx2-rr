#pragma once

#include "EmulatorConfig.h"
#include "GuiConfig.h"

class GlobalConfig
{
public:
	std::unique_ptr<GuiConfig> gui;
	std::unique_ptr<Pcsx2Config> emulator;

	GlobalConfig();
};

extern std::unique_ptr<GlobalConfig> g_Conf;

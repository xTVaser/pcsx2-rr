#include "PrecompiledHeader.h"

#include "GlobalConfig.h"

std::unique_ptr<GlobalConfig> g_Conf;

GlobalConfig::GlobalConfig()
{
	gui = new GuiConfig();
	emulator = new Pcsx2Config();
}

GlobalConfig::~GlobalConfig()
{
	delete gui;
	delete emulator;
}
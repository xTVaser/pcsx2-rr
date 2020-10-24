#include "PrecompiledHeader.h"

#include "GlobalConfig.h"

std::unique_ptr<GlobalConfig> g_Conf;

GlobalConfig::GlobalConfig()
{
	gui = std::make_unique<GuiConfig>();
	emulator = std::make_unique<Pcsx2Config>();
}

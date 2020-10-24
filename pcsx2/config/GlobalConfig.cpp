#include "PrecompiledHeader.h"

#include "GlobalConfig.h"

GlobalConfig::GlobalConfig()
{
	gui = std::make_unique<GuiConfig>();
	emulator = std::make_unique<Pcsx2Config>();
}

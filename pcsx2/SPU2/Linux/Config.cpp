/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2020  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"
#include "../Global.h"
#include "Dialogs.h"
#include "Config.h"
#include "AppConfig.h"

#if defined(__unix__) || defined(__APPLE__)
#include <SDL.h>
#include <SDL_audio.h>
#include "../wx/wxConfig.h"
#endif

int AutoDMAPlayRate[2] = {0, 0};

YamlUtils spuConfig;

// Default settings.

// MIXING
int Interpolation = 4;
/* values:
		0: no interpolation (use nearest)
		1. linear interpolation
		2. cubic interpolation
		3. hermite interpolation
		4. catmull-rom interpolation
*/

bool EffectsDisabled = false;
float FinalVolume; // global
bool AdvancedVolumeControl;
float VolumeAdjustFLdb; // decibels settings, cos audiophiles love that
float VolumeAdjustCdb;
float VolumeAdjustFRdb;
float VolumeAdjustBLdb;
float VolumeAdjustBRdb;
float VolumeAdjustSLdb;
float VolumeAdjustSRdb;
float VolumeAdjustLFEdb;
float VolumeAdjustFL; // linear coefs calculated from decibels,
float VolumeAdjustC;
float VolumeAdjustFR;
float VolumeAdjustBL;
float VolumeAdjustBR;
float VolumeAdjustSL;
float VolumeAdjustSR;
float VolumeAdjustLFE;
unsigned int delayCycles;

bool postprocess_filter_enabled = true;
bool postprocess_filter_dealias = false;
bool _visual_debug_enabled = false; // windows only feature

// OUTPUT
u32 OutputModule = 0;
int SndOutLatencyMS = 300;
int SynchMode = 0; // Time Stretch, Async or Disabled
#ifdef SPU2X_PORTAUDIO
u32 OutputAPI = 0;
#endif
u32 SdlOutputAPI = 0;

int numSpeakers = 0;
int dplLevel = 0;
bool temp_debug_state;

std::string path("SPU2.yaml");
bool pathSet = false;

void init()
{
	if (!pathSet)
	{
		path = Path::Combine(GetSettingsFolder().string(), path);
		pathSet = true;
	}
}

/*****************************************************************************/

void ReadSettings()
{
	// For some reason this can be called before we know what ini file we're writing to.
	// Lets not try to read it if that happens.
	if (!pathSet)
	{
	    init();
	}

	Interpolation = spuConfig.GetStream()["MIXING"]["Interpolation"].as<int>();
	EffectsDisabled = spuConfig.GetStream()["MIXING"]["Disable_Effects"].as<bool>();
	postprocess_filter_dealias = spuConfig.GetStream()["MIXING"]["DealiasFilter"].as<bool>();
	FinalVolume = ((float)spuConfig.GetStream()["MIXING"]["FinalVolume"].as<int>(), 100) / 100;
	if (FinalVolume > 1.0f)
		FinalVolume = 1.0f;

	AdvancedVolumeControl = spuConfig.GetStream()["MIXING"]["AdvancedVolumeControl"].as<bool>();
	VolumeAdjustCdb = spuConfig.GetStream()["MIXING"]["VolumeAdjustC(dB)"].as<float>();
	VolumeAdjustFLdb = spuConfig.GetStream()["MIXING"]["VolumeAdjustFL(dB)"].as<float>();
	VolumeAdjustFRdb = spuConfig.GetStream()["MIXING"]["VolumeAdjustFR(dB)"].as<float>();
	VolumeAdjustBLdb = spuConfig.GetStream()["MIXING"]["VolumeAdjustBL(dB)"].as<float>();
	VolumeAdjustBRdb = spuConfig.GetStream()["MIXING"]["VolumeAdjustBR(dB)"].as<float>();
	VolumeAdjustSLdb = spuConfig.GetStream()["MIXING"]["VolumeAdjustSL(dB)"].as<float>();
	VolumeAdjustSRdb = spuConfig.GetStream()["MIXING"]["VolumeAdjustSR(dB)"].as<float>();
	VolumeAdjustLFEdb = spuConfig.GetStream()["MIXING"]["VolumeAdjustLFE(dB)"].as<float>();
	VolumeAdjustC = powf(10, VolumeAdjustCdb / 10);
	VolumeAdjustFL = powf(10, VolumeAdjustFLdb / 10);
	VolumeAdjustFR = powf(10, VolumeAdjustFRdb / 10);
	VolumeAdjustBL = powf(10, VolumeAdjustBLdb / 10);
	VolumeAdjustBR = powf(10, VolumeAdjustBRdb / 10);
	VolumeAdjustSL = powf(10, VolumeAdjustSLdb / 10);
	VolumeAdjustSR = powf(10, VolumeAdjustSRdb / 10);
	VolumeAdjustLFE = powf(10, VolumeAdjustLFEdb / 10);
	delayCycles = spuConfig.GetStream()["DEBUG"]["DelayCycles"].as<int>();
	wxString temp;

#if SDL_MAJOR_VERSION >= 2 || !defined(SPU2X_PORTAUDIO)
	//CfgReadStr(L"OUTPUT", L"Output_Module", temp, SDLOut->GetIdent());
#else
	//CfgReadStr(L"OUTPUT", L"Output_Module", temp, PortaudioOut->GetIdent());
#endif
	OutputModule = FindOutputModuleById(temp.ToStdString()); // find the driver index of this module

// find current API
#ifdef SPU2X_PORTAUDIO
#ifdef __linux__
	//CfgReadStr(L"PORTAUDIO", L"HostApi", temp, L"ALSA");
	if (temp == L"OSS")
		OutputAPI = 1;
	else if (temp == L"JACK")
		OutputAPI = 2;
	else // L"ALSA"
		OutputAPI = 0;
#else
	CfgReadStr(L"PORTAUDIO", L"HostApi", temp, L"OSS");
	OutputAPI = 0; // L"OSS"
#endif
#endif

#if defined(__unix__) || defined(__APPLE__)
	//CfgReadStr(L"SDL", L"HostApi", temp, L"pulseaudio");
	SdlOutputAPI = 0;
#if SDL_MAJOR_VERSION >= 2
	// YES It sucks ...
	for (int i = 0; i < SDL_GetNumAudioDrivers(); ++i)
	{
		if (!temp.Cmp(wxString(SDL_GetAudioDriver(i), wxConvUTF8)))
			SdlOutputAPI = i;
	}
#endif
#endif

	SndOutLatencyMS = spuConfig.GetStream()["OUTPUT"]["Latency"].as<int>();
	SynchMode = spuConfig.GetStream()["OUTPUT"]["Synch_Mode"].as<int>();
	numSpeakers = spuConfig.GetStream()["OUTPUT"]["SpeakerConfiguration"].as<int>();

#ifdef SPU2X_PORTAUDIO
	PortaudioOut->ReadSettings();
#endif
#if defined(__unix__) || defined(__APPLE__)
	SDLOut->ReadSettings();
#endif
	SoundtouchCfg::ReadSettings();
	DebugConfig::ReadSettings();

	// Sanity Checks
	// -------------

	Clampify(SndOutLatencyMS, LATENCY_MIN, LATENCY_MAX);

	if (mods[OutputModule] == nullptr)
	{
		fwprintf(stderr, L"* SPU2: Unknown output module '%s' specified in configuration file.\n", temp.wc_str());
		//fprintf(stderr, "* SPU2: Defaulting to SDL.\n", SDLOut->GetIdent());
		OutputModule = FindOutputModuleById(SDLOut->GetIdent());
	}

	WriteSettings();
	//spuConfig->Flush();
}

/*****************************************************************************/

void WriteSettings()
{
	if (!pathSet)
	{
		FileLog("Write called without the path set.\n");
		return;
	}

	std::string data;


	YAML::Node Mixing;

	Mixing["Interpolation"] = Interpolation;
	Mixing["Disable_Effects"] = EffectsDisabled;
	Mixing["DealiasFilter"] = postprocess_filter_dealias;
	Mixing["FinalVolume"] = (int)(FinalVolume * 100 + 0.5f);

	Mixing["AdvancedVolumeControl"] = AdvancedVolumeControl;
	Mixing["VolumeAdjustC(dB)"] = VolumeAdjustCdb;
	Mixing["VolumeAdjustFL(dB)"] = VolumeAdjustFLdb;
	Mixing["VolumeAdjustFR(dB)"] = VolumeAdjustFRdb;
	Mixing["VolumeAdjustBL(dB)"] = VolumeAdjustBLdb;
	Mixing["VolumeAdjustBR(dB)"] = VolumeAdjustBRdb;
	Mixing["VolumeAdjustSL(dB)"] = VolumeAdjustSLdb;
	Mixing["VolumeAdjustSR(dB)"] = VolumeAdjustSRdb;
	Mixing["VolumeAdjustLFE(dB)"] = VolumeAdjustLFEdb;

	YAML::Node Output;

	Output["Output_Module"] = mods[OutputModule]->GetIdent();
	Output["Latency"] = SndOutLatencyMS;
	Output["Synch_Mode"] = SynchMode;
	Output["SpeakerConfiguration"] = numSpeakers;
	Output["DelayCycles"] = delayCycles;

#ifdef SPU2X_PORTAUDIO
spuConfig.GetStream()["PortAudio"] = PortaudioOut->WriteSettings();
#endif
#if defined(__unix__) || defined(__APPLE__)
	spuConfig.GetStream()["SDL"] = SDLOut->WriteSettings();
#endif
    spuConfig.GetStream()["SoundTouch"] = SoundtouchCfg::WriteSettings();
    DebugConfig::WriteSettings();


	spuConfig.GetStream()["MIXING"] = Mixing;
	spuConfig.GetStream()["OUTPUT"] = Output;

	std::ostringstream os;
	os << spuConfig.GetStream();
	data = os.str();

	spuConfig.Save(path, data);
}

void configure()
{
	auto* dialog = new Dialog;

	init();
	//ReadSettings();
	dialog->Display();
	WriteSettings();
	wxDELETE(dialog);
}

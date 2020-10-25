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
#include "GuiConfig.h"

#if defined(__unix__) || defined(__APPLE__)
#include <SDL.h>
#include <SDL_audio.h>
#include "../wx/wxConfig.h"
#endif

int AutoDMAPlayRate[2] = {0, 0};

YamlFile spuConfig;

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

void setDefaults()
{

	YAML::Node def;

	Interpolation = 4;
	EffectsDisabled = false;
	postprocess_filter_dealias = false;
	FinalVolume = 50;

	AdvancedVolumeControl = false;
	VolumeAdjustCdb = 0;
	VolumeAdjustFLdb = 0;
	VolumeAdjustFRdb = 0;
	VolumeAdjustBLdb = 0;
	VolumeAdjustBRdb = 0;
	VolumeAdjustSLdb = 0;
	VolumeAdjustSRdb = 0;
	VolumeAdjustLFEdb = 0;
	delayCycles = 4;
	VolumeAdjustC = powf(10, VolumeAdjustCdb / 10);
	VolumeAdjustFL = powf(10, VolumeAdjustFLdb / 10);
	VolumeAdjustFR = powf(10, VolumeAdjustFRdb / 10);
	VolumeAdjustBL = powf(10, VolumeAdjustBLdb / 10);
	VolumeAdjustBR = powf(10, VolumeAdjustBRdb / 10);
	VolumeAdjustSL = powf(10, VolumeAdjustSLdb / 10);
	VolumeAdjustSR = powf(10, VolumeAdjustSRdb / 10);
	VolumeAdjustLFE = powf(10, VolumeAdjustLFEdb / 10);
	SynchMode = 0;
	numSpeakers = 0;
	dplLevel = 0;
	SndOutLatencyMS = 100;

	char omodid[128];

	// find the driver index of this module:
	//OutputModule = FindOutputModuleById(omodid);

	def["MIXING"]["Interpolation"] = Interpolation;
	def["MIXING"]["Disable_Effects"] = EffectsDisabled;
	def["MIXING"]["DealiasFilter"] = postprocess_filter_dealias;
	def["MIXING"]["FinalVolume"] = FinalVolume;
	def["MIXING"]["AdvancedVolumeControl"] = AdvancedVolumeControl;
	def["MIXING"]["VolumeAdjustC(dB)"] = VolumeAdjustCdb;
	def["MIXING"]["VolumeAdjustFL(dB)"] = VolumeAdjustFLdb;
	def["MIXING"]["VolumeAdjustFR(dB)"] = VolumeAdjustFRdb;
	def["MIXING"]["VolumeAdjustBL(dB)"] = VolumeAdjustBLdb;
	def["MIXING"]["VolumeAdjustBR(dB)"] = VolumeAdjustBRdb;
	def["MIXING"]["VolumeAdjustSL(dB)"] = VolumeAdjustSLdb;
	def["MIXING"]["VolumeAdjustSR(dB)"] = VolumeAdjustSRdb;
	def["MIXING"]["VolumeAdjustLFE(dB)"] = VolumeAdjustLFEdb;
	def["DEBUG"]["DelayCycles"] = delayCycles;

	def["OUTPUT"]["Synch_Mode"] = SynchMode;
	def["OUTPUT"]["SpeakerConfiguration"] = numSpeakers;
	def["OUTPUT"]["DplDecodingLevel"] = dplLevel;
	def["OUTPUT"]["Latency"] = SndOutLatencyMS;

	std::ofstream fileStream(path);
	fileStream << def;

	std::string data;

	std::ostringstream os;
	os << def;
	data = os.str();

	std::cout << "Defaults: " << data << std::endl;


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

	if (spuConfig.loadFromFile(path))
    {
		std::shared_ptr<YamlFile> mixingSection = spuConfig.getSection("MIXING");
		Interpolation = mixingSection->getInt("Interpolation", Interpolation);
		EffectsDisabled = mixingSection->getBool("Disable_Effects", EffectsDisabled);
		postprocess_filter_dealias = mixingSection->getBool("DealiasFilter", postprocess_filter_dealias);
		FinalVolume = mixingSection->getFloat("Interpolation", FinalVolume);
	    if (FinalVolume > 1.0f)
		    FinalVolume = 1.0f;

		AdvancedVolumeControl = mixingSection->getBool("AdvancedVolumeControl", AdvancedVolumeControl);
		VolumeAdjustCdb = mixingSection->getFloat("VolumeAdjustC(dB)", VolumeAdjustCdb);
		VolumeAdjustFLdb = mixingSection->getFloat("VolumeAdjustFL(dB)", VolumeAdjustFLdb);
		VolumeAdjustFRdb = mixingSection->getFloat("VolumeAdjustFR(dB)", VolumeAdjustFRdb);
		VolumeAdjustBLdb = mixingSection->getFloat("VolumeAdjustBL(dB)", VolumeAdjustBLdb);
		VolumeAdjustBRdb = mixingSection->getFloat("VolumeAdjustBR(dB)", VolumeAdjustBRdb);
		VolumeAdjustSLdb = mixingSection->getFloat("VolumeAdjustSL(dB)", VolumeAdjustSLdb);
		VolumeAdjustSRdb = mixingSection->getFloat("VolumeAdjustSR(dB)", VolumeAdjustSRdb);
		VolumeAdjustLFEdb = mixingSection->getFloat("VolumeAdjustLFE(dB)", VolumeAdjustLFEdb);
	    VolumeAdjustC = powf(10, VolumeAdjustCdb / 10);
	    VolumeAdjustFL = powf(10, VolumeAdjustFLdb / 10);
	    VolumeAdjustFR = powf(10, VolumeAdjustFRdb / 10);
	    VolumeAdjustBL = powf(10, VolumeAdjustBLdb / 10);
	    VolumeAdjustBR = powf(10, VolumeAdjustBRdb / 10);
	    VolumeAdjustSL = powf(10, VolumeAdjustSLdb / 10);
	    VolumeAdjustSR = powf(10, VolumeAdjustSRdb / 10);
	    VolumeAdjustLFE = powf(10, VolumeAdjustLFEdb / 10);
	    delayCycles = mixingSection->getInt("DelayCycles", delayCycles);
	    std::string temp;

    #if SDL_MAJOR_VERSION >= 2 || !defined(SPU2X_PORTAUDIO)
	    //CfgReadStr(L"OUTPUT", L"Output_Module", temp, SDLOut->GetIdent());
    #else
	    //CfgReadStr(L"OUTPUT", L"Output_Module", temp, PortaudioOut->GetIdent());
    #endif
	    OutputModule = FindOutputModuleById(temp); // find the driver index of this module

    // find current API
    #ifdef SPU2X_PORTAUDIO
    #ifdef __linux__
	    //CfgReadStr(L"PORTAUDIO", L"HostApi", temp, L"ALSA");
	    if (temp == "OSS")
		    OutputAPI = 1;
	    else if (temp == "JACK")
		    OutputAPI = 2;
	    else // L"ALSA"
		    OutputAPI = 0;
	#else
		//CfgReadStr("PORTAUDIO", "HostApi", temp, "OSS");
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
		    if (!temp.compare(SDL_GetAudioDriver(i)) == 0)
			    SdlOutputAPI = i;
	    }
    #endif
    #endif

	    SndOutLatencyMS = mixingSection->getInt("Latency", SndOutLatencyMS);
	    SynchMode = mixingSection->getInt("Synch_Mode", SynchMode);
	    numSpeakers = mixingSection->getInt("SpeakerConfiguration", SynchMode);
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
		    printf("* SPU2: Unknown output module specified in configuration file.\n");
		    //printf(stderr, "* SPU2: Defaulting to SDL.\n", SDLOut->GetIdent());
		    OutputModule = FindOutputModuleById(SDLOut->GetIdent());
	    }

		WriteSettings();
	}

	else
	{
		setDefaults();
	}
	

	spuConfig.saveToFile(path);
}

/*****************************************************************************/

void WriteSettings()
{
	if (!pathSet)
	{
		FileLog("Write called without the path set.\n");
		return;
	}

	spuConfig.loadFromFile(path);

	std::string data;


	std::shared_ptr<YamlFile> mixingSection = spuConfig.getSection("MIXING");
	mixingSection->setInt("Interpolation", Interpolation);
	mixingSection->setBool("Disable_Effects", EffectsDisabled);
	mixingSection->setBool("DealiasFilter", postprocess_filter_dealias);
	mixingSection->setFloat("FinalVolume", FinalVolume);

	mixingSection->setBool("AdvancedVolumeControl", FinalVolume);
	mixingSection->setFloat("VolumeAdjustC(dB)", VolumeAdjustCdb);
	mixingSection->setFloat("VolumeAdjustFL(dB)", VolumeAdjustFLdb);
	mixingSection->setFloat("VolumeAdjustFR(dB)", VolumeAdjustFRdb);
	mixingSection->setFloat("VolumeAdjustBL(dB)", VolumeAdjustBLdb);
	mixingSection->setFloat("VolumeAdjustBR(dB)", VolumeAdjustBRdb);
	mixingSection->setFloat("VolumeAdjustSL(dB)", VolumeAdjustSLdb);
	mixingSection->setFloat("VolumeAdjustSR(dB)", VolumeAdjustSRdb);
	mixingSection->setFloat("VolumeAdjustLFE(dB)", VolumeAdjustLFEdb);
	spuConfig.setSection("MIXING", mixingSection);

	std::shared_ptr<YamlFile> outputSection = spuConfig.getSection("OUTPUT");
	outputSection->setString("Output_Module", mods[OutputModule]->GetIdent());
	outputSection->setInt("Latency", SndOutLatencyMS);
	outputSection->setInt("Synch_Mode", SynchMode);
	outputSection->setInt("SpeakerConfiguration", numSpeakers);
	outputSection->setInt("DplDecodingLevel", dplLevel);
	outputSection->setU32("DelayCycles", delayCycles);
	spuConfig.setSection("OUTPUT", outputSection);

#ifdef SPU2X_PORTAUDIO
//spuConfig.GetStream()["PortAudio"] = PortaudioOut->WriteSettings();
#endif
#if defined(__unix__) || defined(__APPLE__)
	//spuConfig.GetStream()["SDL"] = SDLOut->WriteSettings();
#endif
    //spuConfig.GetStream()["SoundTouch"] = SoundtouchCfg::WriteSettings();
    DebugConfig::WriteSettings();


	//spuConfig.GetStream()["MIXING"] = Mixing;
	//spuConfig.GetStream()["OUTPUT"] = Output;

	std::ostringstream os;
	//os << spuConfig.GetStream();
	data = os.str();

	//std::cout << "Data: " << data << std::endl;
	
	std::ofstream fileStream(path);
	//fileStream << spuConfig.GetStream();
	fileStream.close();
}

void configure()
{
	auto* dialog = new Dialog;

	init();
	ReadSettings();
	dialog->Display();
	WriteSettings();
	wxDELETE(dialog);
}

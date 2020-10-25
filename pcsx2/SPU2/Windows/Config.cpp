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
#include "GuiConfig.h"
#include "Dialogs.h"
#include <math.h>

#include "fmt/core.h"
#include "Utilities/YamlFile.h"

YamlFile spuConfig;
std::string path;
bool pathSet = false;

void init()
{
	if (!pathSet)
	{
		path = Path::Combine(GetSettingsFolder().string(),
							 fmt::format("{}.{}", path, spuConfig.fileExtension()));
		pathSet = true;
	}
}

#ifdef PCSX2_DEVBUILD
static const int LATENCY_MAX = 3000;
#else
static const int LATENCY_MAX = 750;
#endif

static const int LATENCY_MIN = 3;
static const int LATENCY_MIN_TS = 15;

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

float FinalVolume; // Global
bool AdvancedVolumeControl;
float VolumeAdjustFLdb; // decibels settings, cos audiophiles love that
float VolumeAdjustCdb;
float VolumeAdjustFRdb;
float VolumeAdjustBLdb;
float VolumeAdjustBRdb;
float VolumeAdjustSLdb;
float VolumeAdjustSRdb;
float VolumeAdjustLFEdb;
float VolumeAdjustFL; // linear coefs calcualted from decibels,
float VolumeAdjustC;
float VolumeAdjustFR;
float VolumeAdjustBL;
float VolumeAdjustBR;
float VolumeAdjustSL;
float VolumeAdjustSR;
float VolumeAdjustLFE;
unsigned int delayCycles;

bool postprocess_filter_enabled = 1;
bool postprocess_filter_dealias = false;

// OUTPUT
int SndOutLatencyMS = 100;
int SynchMode = 0; // Time Stretch, Async or Disabled

u32 OutputModule = 0;

CONFIG_WAVEOUT Config_WaveOut;
CONFIG_XAUDIO2 Config_XAudio2;

// DSP
bool dspPluginEnabled = false;
int dspPluginModule = 0;
std::string dspPlugin;

int numSpeakers = 0;

int dplLevel = 0;

void setDefaults()
{
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

	if ((SynchMode == 0) && (SndOutLatencyMS < LATENCY_MIN_TS)) // can't use low-latency with timestretcher atm
		SndOutLatencyMS = LATENCY_MIN_TS;
	else if (SndOutLatencyMS < LATENCY_MIN)
		SndOutLatencyMS = LATENCY_MIN;

	char omodid[128];

	// find the driver index of this module:
	OutputModule = FindOutputModuleById(omodid);

	dspPluginModule = 0;
	dspPluginEnabled = false;
	Config_WaveOut.NumBuffers = 4;
}

/*****************************************************************************/

void ReadSettings()
{
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
		// TODO - config - duplicate code
		VolumeAdjustC = powf(10, VolumeAdjustCdb / 10);
		VolumeAdjustFL = powf(10, VolumeAdjustFLdb / 10);
		VolumeAdjustFR = powf(10, VolumeAdjustFRdb / 10);
		VolumeAdjustBL = powf(10, VolumeAdjustBLdb / 10);
		VolumeAdjustBR = powf(10, VolumeAdjustBRdb / 10);
		VolumeAdjustSL = powf(10, VolumeAdjustSLdb / 10);
		VolumeAdjustSR = powf(10, VolumeAdjustSRdb / 10);
		VolumeAdjustLFE = powf(10, VolumeAdjustLFEdb / 10);

		std::shared_ptr<YamlFile> debugSection = spuConfig.getSection("DEBUG");
		delayCycles = debugSection->getInt("DelayCycles", delayCycles);

		std::shared_ptr<YamlFile> outputSection = spuConfig.getSection("OUTPUT");
		SynchMode = outputSection->getInt("Synch_Mode", SynchMode);
		numSpeakers = outputSection->getInt("SpeakerConfiguration", numSpeakers);
		dplLevel = outputSection->getInt("DplDecodingLevel", dplLevel);
		SndOutLatencyMS = outputSection->getInt("Latency", SndOutLatencyMS);

		if ((SynchMode == 0) && (SndOutLatencyMS < LATENCY_MIN_TS)) // can't use low-latency with timestretcher atm
			SndOutLatencyMS = LATENCY_MIN_TS;
		else if (SndOutLatencyMS < LATENCY_MIN)
			SndOutLatencyMS = LATENCY_MIN;

		std::string omodid;

		// portaudio occasionally has issues selecting the proper default audio device.
		// let's use xaudio2 until this is sorted (rama)

		// find the driver index of this module:
		OutputModule = FindOutputModuleById(omodid);

		std::shared_ptr<YamlFile> dspPluginSection = spuConfig.getSection("DSP_PLUGIN");
		dspPluginModule = dspPluginSection->getInt("ModuleNum", dspPluginModule);
		dspPluginEnabled = dspPluginSection->getBool("Enabled", dspPluginEnabled);

		// Read WAVEOUT configs:
		std::shared_ptr<YamlFile> waveOutSection = spuConfig.getSection("WAVEOUT");
		Config_WaveOut.Device = waveOutSection->getString("Device", Config_WaveOut.Device);
		Config_WaveOut.NumBuffers = waveOutSection->getInt("Buffer_Count", Config_WaveOut.NumBuffers); // TODO - config, not technically the right type, char?

		PortaudioOut->ReadSettings();
		SoundtouchCfg::ReadSettings();
		DebugConfig::ReadSettings();

		// Sanity Checks
		// -------------

		Clampify(SndOutLatencyMS, LATENCY_MIN, LATENCY_MAX);

		if (mods[OutputModule] == nullptr)
		{
			// Unsupported or legacy module.
			//fwprintf(stderr, L"* SPU2: Unknown output module '%s' specified in configuration file.\n", omodid);
		}
	}

	else
	{
		setDefaults();
	}
}

/*****************************************************************************/

void WriteSettings()
{
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

	std::shared_ptr<YamlFile> waveoutSection = spuConfig.getSection("WAVEOUT");
	if (Config_WaveOut.Device.empty())
		Config_WaveOut.Device = "default";
	waveoutSection->setString("Device", Config_WaveOut.Device);
	waveoutSection->setInt("Buffer_Count", Config_WaveOut.NumBuffers); // TODO - config, not technically the right type, char?
	spuConfig.setSection("WAVEOUT", waveoutSection);

	std::shared_ptr<YamlFile> dspPluginSection = spuConfig.getSection("DSP_PLUGIN");
	dspPluginSection->getString("Filename", dspPlugin);
	dspPluginSection->getInt("ModuleNum", dspPluginModule);
	dspPluginSection->getBool("Enabled", dspPluginEnabled);
	spuConfig.setSection("DSP_PLUGIN", dspPluginSection);

	// TODO - config - all of these functions still return YAML::Nodes and they are blank!
	/*spuConfig.GetStream()["PortAudio"] = PortaudioOut->WriteSettings();
	spuConfig.GetStream()["SoundTouch"] =
		SoundtouchCfg::WriteSettings();*/

	// TODO - config - this still seems to use the old ini system!
	// DebugConfig::WriteSettings();

	spuConfig.saveToFile(path);
}

void CheckOutputModule(HWND window)
{
	OutputModule = SendMessage(GetDlgItem(window, IDC_OUTPUT), CB_GETCURSEL, 0, 0);
	const bool IsConfigurable =
		mods[OutputModule] == PortaudioOut ||
		mods[OutputModule] == WaveOut;

	const bool AudioExpansion =
		mods[OutputModule] == XAudio2Out ||
		mods[OutputModule] == PortaudioOut;

	EnableWindow(GetDlgItem(window, IDC_OUTCONF), IsConfigurable);
	EnableWindow(GetDlgItem(window, IDC_SPEAKERS), AudioExpansion);
	EnableWindow(GetDlgItem(window, IDC_SPEAKERS_TEXT), AudioExpansion);
}

BOOL CALLBACK ConfigProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	wchar_t temp[384] = {0};

	switch (uMsg)
	{
		case WM_PAINT:
			return FALSE;

		case WM_INITDIALOG:
		{
			SendDialogMsg(hWnd, IDC_INTERPOLATE, CB_RESETCONTENT, 0, 0);
			SendDialogMsg(hWnd, IDC_INTERPOLATE, CB_ADDSTRING, 0, (LPARAM)L"0 - Nearest (Fastest/bad quality)");
			SendDialogMsg(hWnd, IDC_INTERPOLATE, CB_ADDSTRING, 0, (LPARAM)L"1 - Linear (Simple/okay sound)");
			SendDialogMsg(hWnd, IDC_INTERPOLATE, CB_ADDSTRING, 0, (LPARAM)L"2 - Cubic (Artificial highs)");
			SendDialogMsg(hWnd, IDC_INTERPOLATE, CB_ADDSTRING, 0, (LPARAM)L"3 - Hermite (Better highs)");
			SendDialogMsg(hWnd, IDC_INTERPOLATE, CB_ADDSTRING, 0, (LPARAM)L"4 - Catmull-Rom (PS2-like/slow)");
			SendDialogMsg(hWnd, IDC_INTERPOLATE, CB_SETCURSEL, Interpolation, 0);

			SendDialogMsg(hWnd, IDC_SYNCHMODE, CB_RESETCONTENT, 0, 0);
			SendDialogMsg(hWnd, IDC_SYNCHMODE, CB_ADDSTRING, 0, (LPARAM)L"TimeStretch (Recommended)");
			SendDialogMsg(hWnd, IDC_SYNCHMODE, CB_ADDSTRING, 0, (LPARAM)L"Async Mix (Breaks some games!)");
			SendDialogMsg(hWnd, IDC_SYNCHMODE, CB_ADDSTRING, 0, (LPARAM)L"None (Audio can skip.)");
			SendDialogMsg(hWnd, IDC_SYNCHMODE, CB_SETCURSEL, SynchMode, 0);

			SendDialogMsg(hWnd, IDC_SPEAKERS, CB_RESETCONTENT, 0, 0);
			SendDialogMsg(hWnd, IDC_SPEAKERS, CB_ADDSTRING, 0, (LPARAM)L"Stereo (None, Default)");
			SendDialogMsg(hWnd, IDC_SPEAKERS, CB_ADDSTRING, 0, (LPARAM)L"Quadrafonic");
			SendDialogMsg(hWnd, IDC_SPEAKERS, CB_ADDSTRING, 0, (LPARAM)L"Surround 5.1");
			SendDialogMsg(hWnd, IDC_SPEAKERS, CB_ADDSTRING, 0, (LPARAM)L"Surround 7.1");
			SendDialogMsg(hWnd, IDC_SPEAKERS, CB_SETCURSEL, numSpeakers, 0);

			SendDialogMsg(hWnd, IDC_OUTPUT, CB_RESETCONTENT, 0, 0);

			int modidx = 0;
			while (mods[modidx] != nullptr)
			{
				//wprintf_s(temp, 72, L"%d - %s", modidx, mods[modidx]->GetLongName());
				SendDialogMsg(hWnd, IDC_OUTPUT, CB_ADDSTRING, 0, (LPARAM)temp);
				++modidx;
			}
			SendDialogMsg(hWnd, IDC_OUTPUT, CB_SETCURSEL, OutputModule, 0);

			double minlat = (SynchMode == 0) ? LATENCY_MIN_TS : LATENCY_MIN;
			int minexp = (int)(pow(minlat + 1, 1.0 / 3.0) * 128.0);
			int maxexp = (int)(pow((double)LATENCY_MAX + 2, 1.0 / 3.0) * 128.0);
			INIT_SLIDER(IDC_LATENCY_SLIDER, minexp, maxexp, 200, 42, 1);

			SendDialogMsg(hWnd, IDC_LATENCY_SLIDER, TBM_SETPOS, TRUE, (int)((pow((double)SndOutLatencyMS, 1.0 / 3.0) * 128.0) + 1));
			swprintf_s(temp, L"%d ms (avg)", SndOutLatencyMS);
			SetWindowText(GetDlgItem(hWnd, IDC_LATENCY_LABEL), temp);

			int configvol = (int)(FinalVolume * 100 + 0.5f);
			INIT_SLIDER(IDC_VOLUME_SLIDER, 0, 100, 10, 42, 1);

			SendDialogMsg(hWnd, IDC_VOLUME_SLIDER, TBM_SETPOS, TRUE, configvol);
			swprintf_s(temp, L"%d%%", configvol);
			SetWindowText(GetDlgItem(hWnd, IDC_VOLUME_LABEL), temp);

			CheckOutputModule(hWnd);

			EnableWindow(GetDlgItem(hWnd, IDC_OPEN_CONFIG_SOUNDTOUCH), (SynchMode == 0));
			EnableWindow(GetDlgItem(hWnd, IDC_OPEN_CONFIG_DEBUG), DebugEnabled);

			SET_CHECK(IDC_EFFECTS_DISABLE, EffectsDisabled);
			SET_CHECK(IDC_DEALIASFILTER, postprocess_filter_dealias);
			SET_CHECK(IDC_DEBUG_ENABLE, DebugEnabled);
			SET_CHECK(IDC_DSP_ENABLE, dspPluginEnabled);
		}
		break;

		case WM_COMMAND:
			wmId = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case IDOK:
				{
					double res = ((int)SendDialogMsg(hWnd, IDC_LATENCY_SLIDER, TBM_GETPOS, 0, 0)) / 128.0;
					SndOutLatencyMS = (int)pow(res, 3.0);
					Clampify(SndOutLatencyMS, LATENCY_MIN, LATENCY_MAX);
					FinalVolume = (float)(SendDialogMsg(hWnd, IDC_VOLUME_SLIDER, TBM_GETPOS, 0, 0)) / 100;
					Interpolation = (int)SendDialogMsg(hWnd, IDC_INTERPOLATE, CB_GETCURSEL, 0, 0);
					OutputModule = (int)SendDialogMsg(hWnd, IDC_OUTPUT, CB_GETCURSEL, 0, 0);
					SynchMode = (int)SendDialogMsg(hWnd, IDC_SYNCHMODE, CB_GETCURSEL, 0, 0);
					numSpeakers = (int)SendDialogMsg(hWnd, IDC_SPEAKERS, CB_GETCURSEL, 0, 0);

					WriteSettings();
					EndDialog(hWnd, 0);
				}
				break;

				case IDCANCEL:
					EndDialog(hWnd, 0);
					break;

				case IDC_OUTPUT:
					if (wmEvent == CBN_SELCHANGE)
					{
						CheckOutputModule(hWnd);
					}
					break;

				case IDC_OUTCONF:
				{
					const int module = (int)SendMessage(GetDlgItem(hWnd, IDC_OUTPUT), CB_GETCURSEL, 0, 0);
					if (mods[module] == nullptr)
						break;
					mods[module]->Configure((uptr)hWnd);
				}
				break;

				case IDC_OPEN_CONFIG_DEBUG:
				{
					// Quick Hack -- DebugEnabled is re-loaded with the DebugConfig's API,
					// so we need to override it here:

					bool dbgtmp = DebugEnabled;
					DebugConfig::OpenDialog();
					DebugEnabled = dbgtmp;
				}
				break;

				case IDC_SYNCHMODE:
				{
					if (wmEvent == CBN_SELCHANGE)
					{
						int sMode = (int)SendDialogMsg(hWnd, IDC_SYNCHMODE, CB_GETCURSEL, 0, 0);
						double minlat = (sMode == 0) ? LATENCY_MIN_TS : LATENCY_MIN;
						int minexp = (int)(pow(minlat + 1, 1.0 / 3.0) * 128.0);
						int maxexp = (int)(pow((double)LATENCY_MAX + 2, 1.0 / 3.0) * 128.0);
						INIT_SLIDER(IDC_LATENCY_SLIDER, minexp, maxexp, 200, 42, 1);

						int curpos = (int)SendMessage(GetDlgItem(hWnd, IDC_LATENCY_SLIDER), TBM_GETPOS, 0, 0);
						double res = pow(curpos / 128.0, 3.0);
						curpos = (int)res;
						swprintf_s(temp, L"%d ms (avg)", curpos);
						SetDlgItemText(hWnd, IDC_LATENCY_LABEL, temp);
						bool soundtouch = sMode == 0;
						EnableWindow(GetDlgItem(hWnd, IDC_OPEN_CONFIG_SOUNDTOUCH), soundtouch);
					}
				}
				break;


				case IDC_OPEN_CONFIG_SOUNDTOUCH:
					SoundtouchCfg::OpenDialog(hWnd);
					break;

					HANDLE_CHECK(IDC_EFFECTS_DISABLE, EffectsDisabled);
					HANDLE_CHECK(IDC_DEALIASFILTER, postprocess_filter_dealias);
					HANDLE_CHECK(IDC_DSP_ENABLE, dspPluginEnabled);
					HANDLE_CHECKNB(IDC_DEBUG_ENABLE, DebugEnabled);
					DebugConfig::EnableControls(hWnd);
					EnableWindow(GetDlgItem(hWnd, IDC_OPEN_CONFIG_DEBUG), DebugEnabled);
					break;

				default:
					return FALSE;
			}
			break;

		case WM_HSCROLL:
		{
			wmEvent = LOWORD(wParam);
			HWND hwndDlg = (HWND)lParam;

			int curpos = HIWORD(wParam);

			switch (wmEvent)
			{
				case TB_LINEUP:
				case TB_LINEDOWN:
				case TB_PAGEUP:
				case TB_PAGEDOWN:
				case TB_TOP:
				case TB_BOTTOM:
					curpos = (int)SendMessage(hwndDlg, TBM_GETPOS, 0, 0);

				case TB_THUMBPOSITION:
				case TB_THUMBTRACK:
					Clampify(curpos,
							 (int)SendMessage(hwndDlg, TBM_GETRANGEMIN, 0, 0),
							 (int)SendMessage(hwndDlg, TBM_GETRANGEMAX, 0, 0));

					SendMessage((HWND)lParam, TBM_SETPOS, TRUE, curpos);

					if (hwndDlg == GetDlgItem(hWnd, IDC_LATENCY_SLIDER))
					{
						double res = pow(curpos / 128.0, 3.0);
						curpos = (int)res;
						swprintf_s(temp, L"%d ms (avg)", curpos);
						SetDlgItemText(hWnd, IDC_LATENCY_LABEL, temp);
					}

					if (hwndDlg == GetDlgItem(hWnd, IDC_VOLUME_SLIDER))
					{
						swprintf_s(temp, L"%d%%", curpos);
						SetDlgItemText(hWnd, IDC_VOLUME_LABEL, temp);
					}
					break;

				default:
					return FALSE;
			}
		}
		break;

		default:
			return FALSE;
	}
	return TRUE;
}

void configure()
{
	INT_PTR ret;
	ReadSettings();
	ret = DialogBoxParam(nullptr, MAKEINTRESOURCE(IDD_CONFIG), GetActiveWindow(), (DLGPROC)ConfigProc, 1);
	if (ret == -1)
	{
		MessageBox(GetActiveWindow(), L"Error Opening the config dialog.", L"OMG ERROR!", MB_OK | MB_SETFOREGROUND);
		return;
	}
	ReadSettings();
}

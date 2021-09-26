/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2021 PCSX2 Dev Team
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

#include "VMManager.h"

#include <atomic>
#include <mutex>
#include <wx/mstream.h>

#include "common/Console.h"
#include "common/FileSystem.h"
#include "common/ScopedGuard.h"
#include "common/StringUtil.h"
#include "common/SettingsWrapper.h"
#include "common/Timer.h"
#include "fmt/core.h"

#include "Counters.h"
#include "CDVD/CDVD.h"
#include "DEV9/DEV9.h"
#include "Elfheader.h"
#include "FW.h"
#include "GS.h"
#include "HostDisplay.h"
#include "HostSettings.h"
#include "IPC.h"
#include "IopBios.h"
#include "MTVU.h"
#include "MemoryCardFile.h"
#include "Patch.h"
#include "PerformanceMetrics.h"
#include "R5900.h"
#include "SPU2/spu2.h"
#include "System/SysThreads.h"
#include "USB/USB.h"
#include "PAD/Host/PAD.h"
#include "Sio.h"

#include "DebugTools/MIPSAnalyst.h"
#include "DebugTools/SymbolMap.h"

#include "common/emitter/tools.h"
#ifdef _M_X86
#include "common/emitter/x86_intrin.h"
#endif

static void LoadSettings();
static void ApplyGameSettings();
static void CheckForConfigChanges(const Pcsx2Config& old_config);
static void UpdateRunningGame(bool force);

static std::string GetCurrentSaveStateFileName(s32 slot);
static bool DoLoadState(const char* filename);
static bool DoSaveState(const char* filename);

static void SetTimerResolutionIncreased(bool enabled);

static std::unique_ptr<SysMainMemory> s_vm_memory;
static std::unique_ptr<SysCpuProviderPack> s_cpu_provider_pack;

static std::atomic<VMState> s_state{VMState::Shutdown};

static std::mutex s_info_mutex;
static std::string s_disc_path;
static u32 s_game_crc;
static std::string s_game_serial;
static std::string s_game_name;
static std::string s_game_summary;
static std::string s_elf_override;
static u32 s_active_game_fixes = 0;
static std::vector<u8> s_widescreen_cheats_data;
static bool s_widescreen_cheats_loaded = false;
static u32 s_mxcsr_saved;

VMState VMManager::GetState()
{
	return s_state.load();
}

void VMManager::SetState(VMState state)
{
	// Some state transitions aren't valid.
	pxAssert(state != VMState::Starting && state != VMState::Shutdown);
	SetTimerResolutionIncreased(state == VMState::Running);
	s_state.store(state);
}

bool VMManager::HasValidVM()
{
	const VMState state = s_state.load();
	return (state == VMState::Running || state == VMState::Paused);
}

std::string VMManager::GetDiscPath()
{
	std::unique_lock lock(s_info_mutex);
	return s_disc_path;
}

u32 VMManager::GetGameCRC()
{
	std::unique_lock lock(s_info_mutex);
	return s_game_crc;
}

std::string VMManager::GetGameSerial()
{
	std::unique_lock lock(s_info_mutex);
	return s_game_serial;
}

std::string VMManager::GetGameName()
{
	std::unique_lock lock(s_info_mutex);
	return s_game_name;
}

std::string VMManager::GetGameSummary()
{
	std::unique_lock lock(s_info_mutex);
	return s_game_summary;
}

bool VMManager::InitializeMemory()
{
	pxAssert(!s_vm_memory && !s_cpu_provider_pack);

#ifdef _M_X86
	x86caps.Identify();
	x86caps.CountCores();
	x86caps.SIMD_EstablishMXCSRmask();
	x86caps.CalculateMHz();
#endif

	s_vm_memory = std::make_unique<SysMainMemory>();
	s_cpu_provider_pack = std::make_unique<SysCpuProviderPack>();

	s_vm_memory->ReserveAll();
	return true;
}

void VMManager::ReleaseMemory()
{
	std::vector<u8>().swap(s_widescreen_cheats_data);
	s_widescreen_cheats_loaded = false;

	s_vm_memory->DecommitAll();
	s_vm_memory->ReleaseAll();
	s_vm_memory.reset();
	s_cpu_provider_pack.reset();
}

SysMainMemory& GetVmMemory()
{
	return *s_vm_memory;
}

SysCpuProviderPack& GetCpuProviders()
{
	return *s_cpu_provider_pack;
}

void LoadSettings()
{
	auto lock = Host::GetSettingsLock();
	SettingsInterface* si = Host::GetSettingsInterface();
	SettingsLoadWrapper slw(*si);
	EmuConfig.LoadSave(slw);
	PAD::LoadConfig(*si);
	ApplyGameSettings();
}

// Load Game Settings found in database
// (game fixes, round modes, clamp modes, etc...)
// Returns number of gamefixes set
void ApplyGameSettings()
{
	s_active_game_fixes = 0;

	const GameDatabaseSchema::GameEntry* game = GameDatabase::FindGame(s_game_serial);
	if (!game)
		return;

	if (game->eeRoundMode != GameDatabaseSchema::RoundMode::Undefined)
	{
		SSE_RoundMode eeRM = (SSE_RoundMode)enum_cast(game->eeRoundMode);
		if (EnumIsValid(eeRM))
		{
			PatchesCon->WriteLn(L"(GameDB) Changing EE/FPU roundmode to %d [%s]", eeRM, EnumToString(eeRM));
			EmuConfig.Cpu.sseMXCSR.SetRoundMode(eeRM);
			s_active_game_fixes++;
		}
	}

	if (game->vuRoundMode != GameDatabaseSchema::RoundMode::Undefined)
	{
		SSE_RoundMode vuRM = (SSE_RoundMode)enum_cast(game->vuRoundMode);
		if (EnumIsValid(vuRM))
		{
			PatchesCon->WriteLn(L"(GameDB) Changing VU0/VU1 roundmode to %d [%s]", vuRM, EnumToString(vuRM));
			EmuConfig.Cpu.sseVUMXCSR.SetRoundMode(vuRM);
			s_active_game_fixes++;
		}
	}

	if (game->eeClampMode != GameDatabaseSchema::ClampMode::Undefined)
	{
		int clampMode = enum_cast(game->eeClampMode);
		PatchesCon->WriteLn(L"(GameDB) Changing EE/FPU clamp mode [mode=%d]", clampMode);
		EmuConfig.Cpu.Recompiler.fpuOverflow = (clampMode >= 1);
		EmuConfig.Cpu.Recompiler.fpuExtraOverflow = (clampMode >= 2);
		EmuConfig.Cpu.Recompiler.fpuFullMode = (clampMode >= 3);
		s_active_game_fixes++;
	}

	if (game->vuClampMode != GameDatabaseSchema::ClampMode::Undefined)
	{
		int clampMode = enum_cast(game->vuClampMode);
		PatchesCon->WriteLn("(GameDB) Changing VU0/VU1 clamp mode [mode=%d]", clampMode);
		EmuConfig.Cpu.Recompiler.vuOverflow = (clampMode >= 1);
		EmuConfig.Cpu.Recompiler.vuExtraOverflow = (clampMode >= 2);
		EmuConfig.Cpu.Recompiler.vuSignOverflow = (clampMode >= 3);
		s_active_game_fixes++;
	}

	// TODO - config - this could be simplified with maps instead of bitfields and enums
	for (SpeedhackId id = SpeedhackId_FIRST; id < pxEnumEnd; id++)
	{
		std::string key = fmt::format("{}SpeedHack", wxString(EnumToString(id)));

		// Gamefixes are already guaranteed to be valid, any invalid ones are dropped
		if (game->speedHacks.count(key) == 1)
		{
			// Legacy note - speedhacks are setup in the GameDB as integer values, but
			// are effectively booleans like the gamefixes
			bool mode = game->speedHacks.at(key) ? 1 : 0;
			EmuConfig.Speedhacks.Set(id, mode);
			PatchesCon->WriteLn(L"(GameDB) Setting Speedhack '" + key + "' to [mode=%d]", mode);
			s_active_game_fixes++;
		}
	}

	// TODO - config - this could be simplified with maps instead of bitfields and enums
	for (GamefixId id = GamefixId_FIRST; id < pxEnumEnd; id++)
	{
		std::string key = fmt::format("{}Hack", wxString(EnumToString(id)));

		// Gamefixes are already guaranteed to be valid, any invalid ones are dropped
		if (std::find(game->gameFixes.begin(), game->gameFixes.end(), key) != game->gameFixes.end())
		{
			// if the fix is present, it is said to be enabled
			EmuConfig.Gamefixes.Set(id, true);
			PatchesCon->WriteLn(L"(GameDB) Enabled Gamefix: " + key);
			s_active_game_fixes++;

			// The LUT is only used for 1 game so we allocate it only when the gamefix is enabled (save 4MB)
			if (id == Fix_GoemonTlbMiss && true)
				vtlb_Alloc_Ppmap();
		}
	}
}

static void LoadPatches(std::stringstream& summary, const std::string& crc_string, bool show_messages, bool show_messages_when_disabled)
{
	FastFormatAscii message;

	// regular cheat patches
	int cheat_count = 0;
	if (EmuConfig.EnableCheats)
	{
		cheat_count = LoadPatchesFromDir(StringUtil::UTF8StringToWxString(crc_string), EmuFolders::Cheats, L"Cheats");
		summary << " [" << cheat_count << " Cheats]";
		if (cheat_count > 0)
			message.Write("%u cheat patches", cheat_count);
	}

	// wide screen patches
	int ws_patch_count = 0;
	if (EmuConfig.EnableWideScreenPatches && s_game_crc != 0)
	{
		if (ws_patch_count = LoadPatchesFromDir(StringUtil::UTF8StringToWxString(crc_string), EmuFolders::CheatsWS, L"Widescreen hacks"))
		{
			Console.WriteLn(Color_Gray, "Found widescreen patches in the cheats_ws folder --> skipping cheats_ws.zip");
		}
		else
		{
			// No ws cheat files found at the cheats_ws folder, try the ws cheats zip file.
			if (!s_widescreen_cheats_loaded)
			{
				std::optional<std::vector<u8>> data = Host::ReadResourceFile("cheats_ws.zip");
				if (data.has_value())
					s_widescreen_cheats_data = std::move(data.value());
			}

			if (!s_widescreen_cheats_data.empty())
			{
				ws_patch_count = LoadPatchesFromZip(StringUtil::UTF8StringToWxString(crc_string), wxT("cheats_ws.zip"), new wxMemoryInputStream(s_widescreen_cheats_data.data(), s_widescreen_cheats_data.size()));
				PatchesCon->WriteLn(Color_Green, "(Wide Screen Cheats DB) Patches Loaded: %d", ws_patch_count);
			}
		}

		summary << " [" << ws_patch_count << " widescreen patches]";

		if (ws_patch_count > 0)
			message.Write("%s%u widescreen patches", (cheat_count > 0) ? " and " : "", ws_patch_count);
	}

	if (show_messages)
	{
		if (cheat_count > 0 || ws_patch_count > 0)
		{
			message.Write(" are active.");
			Host::AddKeyedOSDMessage("LoadPatches", message.GetString().ToStdString(), 10.0f);
		}
		else if (show_messages_when_disabled)
		{
			Host::AddKeyedOSDMessage("LoadPatches", "No cheats or widescreen patches are found/enabled.", 10.0f);
		}
	}
}

void UpdateRunningGame(bool force)
{
	std::unique_lock lock(s_info_mutex);

	// The CRC can be known before the game actually starts (at the bios), so when
	// we have the CRC but we're still at the bios and the settings are changed
	// (e.g. the user presses TAB to speed up emulation), we don't want to apply the
	// settings as if the game is already running (title, loadeding patches, etc).
	bool ingame = (ElfCRC && (g_GameLoading || g_GameStarted));
	const u32 new_crc = ingame ? ElfCRC : 0;
	const std::string crc_string(StringUtil::StdStringFromFormat("%08X", new_crc));

	std::string new_serial(ingame ? SysGetDiscID().ToStdString() : SysGetBiosDiscID().ToStdString());
	const bool verbose(new_serial != s_game_serial && ingame);
	// SetupPatchesCon(verbose);

	if (!force && s_game_crc == new_crc && s_game_serial == new_serial)
		return;

	s_game_serial = std::move(new_serial);
	s_game_crc = new_crc;

	const Pcsx2Config old_config(EmuConfig);
	ForgetLoadedPatches();
	ApplyGameSettings();

	s_game_name.clear();

	std::stringstream summary;

	if (const GameDatabaseSchema::GameEntry* game = GameDatabase::FindGame(s_game_serial))
	{
		s_game_name = game->name;

		summary << game->name;
		summary << " (" << game->region << ")";
		summary << " [" << s_game_serial << "]";
		summary << " [" << crc_string << "]";
		summary << " [Status = " << GameDatabaseSchema::compatToString(game->compat) << "]";

		if (EmuConfig.EnablePatches)
		{
			if (int patches = LoadPatchesFromGamesDB(crc_string, *game))
			{
				summary << " [" << patches << " Patches]";
				PatchesCon->WriteLn(Color_Green, "(GameDB) Patches Loaded: %d", patches);
			}
			if (s_active_game_fixes > 0)
				summary << " [" << s_active_game_fixes << " Fixes]";
		}

		sioSetGameSerial(game->MemcardFiltersAsString());
	}
	else
	{
		if (s_game_serial.empty() && s_game_crc == 0)
		{
			s_game_name = "Booting PS2 BIOS...";
			summary << s_game_name;
		}
		else
		{
			s_game_name.clear();
			summary << " [" << s_game_serial << "]";
			summary << " [" << crc_string << "]";
		}

		sioSetGameSerial(s_game_serial);
	}

	LoadPatches(summary, crc_string, true, false);

	// When we're booting, the bios loader will set a a title which would be more interesting than this
	// to most users - with region, version, etc, so don't overwrite it with patch info. That's OK. Those
	// users which want to know the status of the patches at the bios can check the console content.
	s_game_summary = summary.str();
	Console.SetTitle(StringUtil::UTF8StringToWxString(s_game_summary));
	Host::GameChanged(s_disc_path, s_game_serial, s_game_name, s_game_crc);
	CheckForConfigChanges(old_config);
}

void VMManager::ReloadPatches(bool verbose)
{
	std::stringstream summary;
	const std::string crc_string(StringUtil::StdStringFromFormat("%08X", s_game_crc));
	ForgetLoadedPatches();
	LoadPatches(summary, crc_string, verbose, verbose);
}

static LimiterModeType GetInitialLimiterMode()
{
	return EmuConfig.GS.FrameLimitEnable ? LimiterModeType::Nominal : LimiterModeType::Unlimited;
}

static void ApplyBootParameters(const VMBootParameters& params)
{
	const bool default_fast_boot = Host::GetBoolSettingValue("EmuCore", "EnableFastBoot", true);
	EmuConfig.UseBOOT2Injection =
		(params.source_type != CDVD_SourceType::NoDisc && params.fast_boot.value_or(default_fast_boot));

	CDVDsys_SetFile(CDVD_SourceType::Iso, params.source);
	CDVDsys_ChangeSource(params.source_type);

	if (!params.elf_override.empty())
	{
		Hle_SetElfPath(params.elf_override.c_str());
		s_elf_override = std::move(params.elf_override);
		EmuConfig.UseBOOT2Injection = true;
	}
	else
	{
		std::string().swap(s_elf_override);
	}

	if (params.source_type == CDVD_SourceType::Iso)
		s_disc_path = params.source;
	else
		s_disc_path.clear();
}

bool VMManager::Initialize(const VMBootParameters& boot_params)
{
	const Common::Timer init_timer;
	pxAssertRel(s_state.load() == VMState::Shutdown, "VM is shutdown");
	s_state.store(VMState::Starting);

	ScopedGuard close_state = [] { s_state.store(VMState::Shutdown); };

	LoadSettings();
	ApplyBootParameters(boot_params);
	EmuConfig.LimiterMode = GetInitialLimiterMode();

	Console.WriteLn("Allocating memory map...");
	s_vm_memory->CommitAll();

	Console.WriteLn("Opening CDVD...");
	if (!DoCDVDopen())
	{
		Console.WriteLn("Failed to initialize CDVD.");
		return false;
	}
	ScopedGuard close_cdvd = [] { DoCDVDclose(); };

	Console.WriteLn("Opening GS...");

	// TODO: Fix up the MTGS crap here.
	static bool gs_initialized = false;
	if (!gs_initialized)
	{
		if (GSinit() != 0)
		{
			Console.WriteLn("Failed to initialize GS.");
			return false;
		}

		gs_initialized = true;
	}
	GetMTGS().WaitForOpen();
	ScopedGuard close_gs = []() { GetMTGS().Suspend(); };

	Console.WriteLn("Opening SPU2...");
	if (SPU2init() != 0 || SPU2open() != 0)
	{
		Console.WriteLn("Failed to initialize SPU2.");
		SPU2shutdown();
		return false;
	}
	ScopedGuard close_spu2 = []() {
		SPU2close();
		SPU2shutdown();
	};

	Console.WriteLn("Opening PAD...");
	if (PADinit() != 0 || PADopen(Host::GetHostDisplay()->GetWindowInfo()) != 0)
	{
		Console.WriteLn("Failed to initialize PAD.");
		return false;
	}
	ScopedGuard close_pad = []() {
		PADclose();
		PADshutdown();
	};

	Console.WriteLn("Opening DEV9...");
	if (DEV9init() != 0 || DEV9open() != 0)
	{
		Console.WriteLn("Failed to initialize DEV9.");
		return false;
	}
	ScopedGuard close_dev9 = []() {
		DEV9close();
		DEV9shutdown();
	};

	Console.WriteLn("Opening USB...");
	if (USBinit() != 0 || USBopen(Host::GetHostDisplay()->GetWindowInfo()) != 0)
	{
		Console.WriteLn("Failed to initialize USB.");
		return false;
	}
	ScopedGuard close_usb = []() {
		USBclose();
		USBshutdown();
	};

	Console.WriteLn("Opening FW...");
	if (FWopen() != 0)
	{
		Console.WriteLn("Failed to initialize FW.");
		return false;
	}
	ScopedGuard close_fw = []() { FWclose(); };

	FileMcd_EmuOpen();

	// Don't close when we return
	close_fw.Cancel();
	close_usb.Cancel();
	close_dev9.Cancel();
	close_pad.Cancel();
	close_spu2.Cancel();
	close_gs.Cancel();
	close_cdvd.Cancel();
	close_state.Cancel();

#if defined(_M_X86)
	s_mxcsr_saved = _mm_getcsr();
#elif defined(_M_ARM64)
	s_mxcsr_saved = static_cast<u32>(a64_getfpcr());
#endif

	// NOTE: Setting the CPU state must come first, because on ARM we compile the rounding
	// mode directly into the microVU dispatcher, rather than loading it from memory.
	SetCPUState(EmuConfig.Cpu.sseMXCSR, EmuConfig.Cpu.sseVUMXCSR);
	SysClearExecutionCache();
	memBindConditionalHandlers();

	ForgetLoadedPatches();
	gsUpdateFrequency(EmuConfig);
	frameLimitReset();
	cpuReset();

	UpdateRunningGame(true);

	PerformanceMetrics::Reset();
	Console.WriteLn("VM subsystems initialized in %.2f ms", init_timer.GetTimeMilliseconds());
	s_state.store(VMState::Paused);

	// do we want to load state?
	if (!boot_params.save_state.empty())
	{
		if (!DoLoadState(boot_params.save_state.c_str()))
		{
			Shutdown();
			return false;
		}
	}

	return true;
}

void VMManager::Shutdown(bool allow_save_resume_state /* = true */)
{
	SetTimerResolutionIncreased(false);

	if (allow_save_resume_state && ShouldSaveResumeState())
	{
		std::string resume_file_name(GetCurrentSaveStateFileName(-1));
		if (!resume_file_name.empty() && !DoSaveState(resume_file_name.c_str()))
			Console.Error("Failed to save resume state");
	}

	{
		std::unique_lock lock(s_info_mutex);
		s_disc_path.clear();
		s_game_crc = 0;
		s_game_serial.clear();
		s_game_name.clear();
		s_game_summary.clear();
		Host::GameChanged(s_disc_path, s_game_serial, s_game_name, 0);
	}

	std::string().swap(s_elf_override);

#ifdef _M_X86
	_mm_setcsr(s_mxcsr_saved);
#elif defined(_M_ARM64)
	a64_setfpcr(s_mxcsr_saved);
#endif

	ForgetLoadedPatches();
	R3000A::ioman::reset();
	USBclose();
	SPU2close();
	PADclose();
	DEV9close();
	DoCDVDclose();
	FWclose();
	FileMcd_EmuClose();
	GetMTGS().Suspend();
	USBshutdown();
	SPU2shutdown();
	PADshutdown();
	DEV9shutdown();

	// GS mess here...
	GetMTGS().Cancel();
	GSshutdown();

	s_vm_memory->DecommitAll();

	s_state.store(VMState::Shutdown);
}

void VMManager::Reset()
{
	const bool game_was_started = g_GameStarted;

	SysClearExecutionCache();
	memBindConditionalHandlers();
	UpdateVSyncRate();
	frameLimitReset();
	cpuReset();

	// gameid change, so apply settings
	if (game_was_started)
		UpdateRunningGame(true);
}

bool VMManager::ShouldSaveResumeState()
{
	return Host::GetBoolSettingValue("EmuCore", "AutoStateLoadSave", false);
}

std::string VMManager::GetSaveStateFileName(const char* game_serial, u32 game_crc, s32 slot)
{
	if (!game_serial || game_serial[0] == '\0')
		return std::string();

	std::string filename;
	if (slot < 0)
		filename = StringUtil::StdStringFromFormat("%s (%08X).resume.p2s", game_serial, game_crc);
	else
		filename = StringUtil::StdStringFromFormat("%s (%08X).%02d.p2s", game_serial, game_crc, slot);

	return Path::CombineStdString(EmuFolders::Savestates, filename);
}

bool VMManager::HasSaveStateInSlot(const char* game_serial, u32 game_crc, s32 slot)
{
	std::string filename(GetSaveStateFileName(game_serial, game_crc, slot));
	return (!filename.empty() && FileSystem::FileExists(filename.c_str()));
}

std::string GetCurrentSaveStateFileName(s32 slot)
{
	std::unique_lock lock(s_info_mutex);
	return VMManager::GetSaveStateFileName(s_game_serial.c_str(), s_game_crc, slot);
}

bool DoLoadState(const char* filename)
{
	try
	{
		SaveState_UnzipFromDisk(wxString::FromUTF8(filename));
		UpdateRunningGame(false);
		return true;
	}
	catch (Exception::BaseException& e)
	{
		Console.WriteLn("Failed to load save state: %s", static_cast<const char*>(e.DiagMsg().c_str()));
		return false;
	}
}

bool DoSaveState(const char* filename)
{
	try
	{
		std::unique_ptr<ArchiveEntryList> elist = std::make_unique<ArchiveEntryList>(new VmStateBuffer(L"Zippable Savestate"));
		SaveState_DownloadState(elist.get());
		SaveState_ZipToDisk(elist.release(), wxString::FromUTF8(filename));
		Host::InvalidateSaveStateCache();
		return true;
	}
	catch (Exception::BaseException& e)
	{
		Console.WriteLn("Failed to save save state: %s", static_cast<const char*>(e.DiagMsg().c_str()));
		return false;
	}
}

bool VMManager::LoadState(const char* filename)
{
	// TODO: Save the current state so we don't need to reset.
	if (DoLoadState(filename))
		return true;

	Reset();
	return false;
}

bool VMManager::LoadStateFromSlot(s32 slot)
{
	const std::string filename(GetCurrentSaveStateFileName(slot));
	if (filename.empty())
		return false;

	Host::AddFormattedOSDMessage(10.0f, "Loading state from slot %d...", slot);
	return DoLoadState(filename.c_str());
}

bool VMManager::SaveState(const char* filename)
{
	return DoSaveState(filename);
}

bool VMManager::SaveStateToSlot(s32 slot)
{
	const std::string filename(GetCurrentSaveStateFileName(slot));
	if (filename.empty())
		return false;

	Host::AddFormattedOSDMessage(10.0f, "Saving state to slot %d...", slot);
	return DoSaveState(filename.c_str());
}

void VMManager::SetLimiterMode(LimiterModeType type)
{
	if (EmuConfig.LimiterMode == type)
		return;

	EmuConfig.LimiterMode = type;
	gsUpdateFrequency(EmuConfig);
	GetMTGS().SetVSync(EmuConfig.GetEffectiveVsyncMode(), EmuConfig.GetPresentFPSLimit());
}

bool VMManager::ChangeDisc(std::string path)
{
	std::string old_path(CDVDsys_GetFile(CDVD_SourceType::Iso));
	CDVD_SourceType old_type = CDVDsys_GetSourceType();

	const std::string display_name(path.empty() ? std::string() : FileSystem::GetDisplayNameFromPath(path));
	CDVDsys_ChangeSource(path.empty() ? CDVD_SourceType::NoDisc : CDVD_SourceType::Iso);
	if (!path.empty())
		CDVDsys_SetFile(CDVD_SourceType::Iso, std::move(path));

	const bool result = DoCDVDopen();
	if (result)
	{
		Host::AddFormattedOSDMessage(10.0f, "Disc changed to '%s'.", display_name.c_str());
	}
	else
	{
		Host::AddFormattedOSDMessage(20.0f, "Failed to open new disc image '%s'. Reverting to old image.", display_name.c_str());
		CDVDsys_ChangeSource(old_type);
		if (!old_path.empty())
			CDVDsys_SetFile(old_type, std::move(old_path));
		if (!DoCDVDopen())
		{
			Host::AddFormattedOSDMessage(20.0f, "Failed to switch back to old disc image. Removing disc.");
			CDVDsys_ChangeSource(CDVD_SourceType::NoDisc);
			DoCDVDopen();
		}
	}

	cdvdCtrlTrayOpen();
	return result;
}

bool VMManager::IsElfFileName(const std::string& path)
{
	const std::string::size_type pos = path.rfind('.');
	if (pos < 0)
		return false;

	return (StringUtil::Strcasecmp(&path[pos], ".elf") == 0);
}

void VMManager::SetBootParametersForPath(const std::string& path, VMBootParameters* params)
{
	if (IsElfFileName(path))
	{
		params->elf_override = path;
	}
	else if (!path.empty())
	{
		params->source_type = CDVD_SourceType::Iso;
		params->source = path;
	}
	else
	{
		params->source_type = CDVD_SourceType::NoDisc;
	}
}

void VMManager::Execute()
{
	Cpu->Execute();
}

void VMManager::SetPaused(bool paused)
{
	if (!HasValidVM())
		return;

	s_state.store(paused ? VMState::Paused : VMState::Running);
	SetTimerResolutionIncreased(paused);
	if (!paused)
	{
		PerformanceMetrics::Reset();
		frameLimitReset();
	}
}

const std::string& VMManager::Internal::GetElfOverride()
{
	return s_elf_override;
}

bool VMManager::Internal::IsExecutionInterrupted()
{
	return s_state.load() != VMState::Running;
}

void VMManager::Internal::GameStartingOnCPUThread()
{
	GetMTGS().SendGameCRC(ElfCRC);

	MIPSAnalyst::ScanForFunctions(R5900SymbolMap, ElfTextRange.first, ElfTextRange.first + ElfTextRange.second, true);
	R5900SymbolMap.UpdateActiveSymbols();
	R3000SymbolMap.UpdateActiveSymbols();

	UpdateRunningGame(false);
	ApplyLoadedPatches(PPT_ONCE_ON_LOAD);
	ApplyLoadedPatches(PPT_COMBINED_0_1);
}

void VMManager::Internal::VSyncOnCPUThread()
{
	// TODO: Move frame limiting here to reduce CPU usage after sleeping...
	ApplyLoadedPatches(PPT_CONTINUOUSLY);
	ApplyLoadedPatches(PPT_COMBINED_0_1);

	Host::PumpMessagesOnCPUThread();
	PAD::PollDevices();
}

static void CheckForCPUConfigChanges(const Pcsx2Config& old_config)
{
	if (EmuConfig.Cpu == old_config.Cpu &&
		EmuConfig.Gamefixes == old_config.Gamefixes &&
		EmuConfig.Speedhacks == old_config.Speedhacks &&
		EmuConfig.Profiler == old_config.Profiler)
	{
		return;
	}

	Console.WriteLn("Updating CPU configuration...");
	SetCPUState(EmuConfig.Cpu.sseMXCSR, EmuConfig.Cpu.sseVUMXCSR);
	SysClearExecutionCache();
	memBindConditionalHandlers();
}

static void CheckForGSConfigChanges(const Pcsx2Config& old_config)
{
	if (EmuConfig.GS == old_config.GS)
		return;

	Console.WriteLn("Updating GS configuration...");

	if (EmuConfig.GS.FrameLimitEnable != old_config.GS.FrameLimitEnable)
		EmuConfig.LimiterMode = GetInitialLimiterMode();

	gsUpdateFrequency(EmuConfig);
	UpdateVSyncRate();
	frameLimitReset();
	GetMTGS().ApplySettings();
	GetMTGS().SetVSync(EmuConfig.GetEffectiveVsyncMode(), EmuConfig.GetPresentFPSLimit());
}

static void CheckForFramerateConfigChanges(const Pcsx2Config& old_config)
{
	if (EmuConfig.Framerate == old_config.Framerate)
		return;

	Console.WriteLn("Updating frame rate configuration");
	gsUpdateFrequency(EmuConfig);
	UpdateVSyncRate();
	frameLimitReset();
	GetMTGS().SetVSync(EmuConfig.GetEffectiveVsyncMode(), EmuConfig.GetPresentFPSLimit());
}

void CheckForConfigChanges(const Pcsx2Config& old_config)
{
	CheckForCPUConfigChanges(old_config);
	CheckForGSConfigChanges(old_config);
	CheckForFramerateConfigChanges(old_config);

	if (EmuConfig.EnableCheats != old_config.EnableCheats || EmuConfig.EnableWideScreenPatches != old_config.EnableWideScreenPatches)
		VMManager::ReloadPatches(true);
}

void VMManager::ApplySettings()
{
	Console.WriteLn("Applying settings...");

	const Pcsx2Config old_config(EmuConfig);
	LoadSettings();
	CheckForConfigChanges(old_config);
}

#ifdef _WIN32

#include "common/RedtapeWindows.h"

static bool s_timer_resolution_increased = false;

void SetTimerResolutionIncreased(bool enabled)
{
	if (s_timer_resolution_increased == enabled)
		return;

	if (enabled)
	{
		s_timer_resolution_increased = (timeBeginPeriod(1) == TIMERR_NOERROR);
	}
	else if (s_timer_resolution_increased)
	{
		timeEndPeriod(1);
		s_timer_resolution_increased = false;
	}
}

#else

void SetTimerResolutionIncreased(bool enabled)
{
}

#endif
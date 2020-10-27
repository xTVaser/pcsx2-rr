#pragma once

#include "Utilities/YamlFile.h"
#include "Utilities/PathUtils.h"
#include "x86emitter/tools.h"

enum PluginsEnum_t
{
	PluginId_GS = 0,
	PluginId_PAD,
	PluginId_USB,
	PluginId_DEV9,
	PluginId_Count,

	// Memorycard plugin support is preliminary, and is only hacked/hardcoded in at this
	// time.  So it's placed afer PluginId_Count so that it doesn't show up in the conf
	// screens or other plugin tables.

	PluginId_Mcd,
	PluginId_AllocCount // Extra value for correct array allocation
};

enum GamefixId
{
	GamefixId_FIRST = 0,

	Fix_VuAddSub = GamefixId_FIRST,
	Fix_FpuCompare,
	Fix_FpuMultiply,
	Fix_FpuNegDiv,
	Fix_XGKick,
	Fix_IpuWait,
	Fix_EETiming,
	Fix_SkipMpeg,
	Fix_OPHFlag,
	Fix_DMABusy,
	Fix_VIFFIFO,
	Fix_VIF1Stall,
	Fix_GIFFIFO,
	Fix_GoemonTlbMiss,
	Fix_ScarfaceIbit,
	Fix_CrashTagTeamIbit,
	Fix_VU0Kickstart,

	GamefixId_COUNT
};

enum class VsyncMode
{
	Off,
	On,
	Adaptive,
};

// Template function for casting enumerations to their underlying type
template <typename Enumeration>
typename std::underlying_type<Enumeration>::type enum_cast(Enumeration E)
{
	return static_cast<typename std::underlying_type<Enumeration>::type>(E);
}

ImplementEnumOperators(GamefixId);

//------------ DEFAULT sseMXCSR VALUES ---------------
#define DEFAULT_sseMXCSR 0xffc0 //FPU rounding > DaZ, FtZ, "chop"
#define DEFAULT_sseVUMXCSR 0xffc0 //VU  rounding > DaZ, FtZ, "chop"

// --------------------------------------------------------------------------------------
//  Pcsx2Config class
// --------------------------------------------------------------------------------------
// This is intended to be a public class library between the core emulator and GUI only.
// It is *not* meant to be shared data between core emulation and plugins, due to issues
// with version incompatibilities if the structure formats are changed.
//
// When GUI code performs modifications of this class, it must be done with strict thread
// safety, since the emu runs on a separate thread.  Additionally many components of the
// class require special emu-side resets or state save/recovery to be applied.  Please
// use the provided functions to lock the emulation into a safe state and then apply
// chances on the necessary scope (see Core_Pause, Core_ApplySettings, and Core_Resume).

// TODO - config - split this up into multiple files, its not fun to work with
struct Pcsx2Config
{
	struct TraceLogFilters
	{
		struct TraceFilters
		{
			bool m_EnableAll = false, // Master Enable switch (if false, no logs at all)
				 m_EnableDisasm = false,
				 m_EnableRegisters = false,
				 m_EnableEvents = false; // Enables logging of event-driven activity -- counters, DMAs, etc.

			void load(std::shared_ptr<YamlFile> configSection);
			std::shared_ptr<YamlFile> save();

			bool operator==(const TraceFilters& right) const;
			bool operator!=(const TraceFilters& right) const;
		};

		// Enabled - global toggle for high volume logging.  This is effectively the equivalent to
		// (EE.Enabled() || IOP.Enabled() || SIF) -- it's cached so that we can use the macros
		// below to inline the conditional check.  This is desirable because these logs are
		// *very* high volume, and debug builds get noticably slower if they have to invoke
		// methods/accessors to test the log enable bits.  Debug builds are slow enough already,
		// so I prefer this to help keep them usable.
		bool EnableTraceLogFilters = false;

		TraceFilters EE;
		TraceFilters IOP;

		void load(std::shared_ptr<YamlFile> configSection);
		std::shared_ptr<YamlFile> save();

		bool operator==(const TraceLogFilters& right) const;
		bool operator!=(const TraceLogFilters& right) const;
	};

	struct ProfilerOptions
	{
		bool
			EnableProfiler = false, // universal toggle for the profiler.
			RecBlocks_EE = false,   // Enables per-block profiling for the EE recompiler [unimplemented]
			RecBlocks_IOP = false,  // Enables per-block profiling for the IOP recompiler [unimplemented]
			RecBlocks_VU0 = false,  // Enables per-block profiling for the VU0 recompiler [unimplemented]
			RecBlocks_VU1 = false;  // Enables per-block profiling for the VU1 recompiler [unimplemented]

		void load(std::shared_ptr<YamlFile> configSection);
		std::shared_ptr<YamlFile> save();

		bool operator==(const ProfilerOptions& right) const;
		bool operator!=(const ProfilerOptions& right) const;
	};

	// ------------------------------------------------------------------------
	struct RecompilerOptions
	{
		bool
			EnableEE = true,
			EnableIOP = true,
			EnableVU0 = false,
			EnableVU1 = true;

		bool
			UseMicroVU0 = true,
			UseMicroVU1 = true;

		bool
			vuOverflow = true,
			vuExtraOverflow = false,
			vuSignOverflow = false,
			vuUnderflow = false;

		bool
			fpuOverflow = true,
			fpuExtraOverflow = false,
			fpuFullMode = false;

		bool
			StackFrameChecks = false,
			PreBlockCheckEE = false,
			PreBlockCheckIOP = false;
		bool
			EnableEECache = false;

		RecompilerOptions();
		void ApplySanityCheck();

		// TODO - saveable interface!

		void load(std::shared_ptr<YamlFile> configSection);
		std::shared_ptr<YamlFile> save();

		bool operator==(const RecompilerOptions& right) const;
		bool operator!=(const RecompilerOptions& right) const;
	};

	// ------------------------------------------------------------------------
	struct CpuOptions
	{
		RecompilerOptions Recompiler;

		SSE_MXCSR sseMXCSR;
		SSE_MXCSR sseVUMXCSR;

		CpuOptions();

		void load(std::shared_ptr<YamlFile> configSection);
		std::shared_ptr<YamlFile> save();

		void ApplySanityCheck();

		bool operator==(const CpuOptions& right) const;
		bool operator!=(const CpuOptions& right) const;
	};

	// ------------------------------------------------------------------------
	struct GSOptions
	{
		// forces the MTGS to execute tags/tasks in fully blocking/synchronous
		// style.  Useful for debugging potential bugs in the MTGS pipeline.
		bool SynchronousMTGS = false;

		int VsyncQueueSize = 2;

		bool FrameLimitEnable = true;
		bool FrameSkipEnable = false;
		VsyncMode VsyncEnable = VsyncMode::Off;

		int FramesToDraw = 2; // number of consecutive frames (fields) to render
		int FramesToSkip = 2; // number of consecutive frames (fields) to skip

		float LimitScalar = 1.0;
		float FramerateNTSC = 59.94;
		float FrameratePAL = 50.0;

		GSOptions();

		void load(std::shared_ptr<YamlFile> configSection);
		std::shared_ptr<YamlFile> save();

		int GetVsync() const;

		bool operator==(const GSOptions& right) const;
		bool operator!=(const GSOptions& right) const;
	};

	// ------------------------------------------------------------------------
	// NOTE: The GUI's GameFixes panel is dependent on the order of bits in this structure.
	struct GamefixOptions
	{
		bool
			VuAddSubHack = false,           // Tri-ace games, they use an encryption algorithm that requires VU ADDI opcode to be bit-accurate.
			FpuCompareHack = false,         // Digimon Rumble Arena 2, fixes spinning/hanging on intro-menu.
			FpuMulHack = false,             // Tales of Destiny hangs.
			FpuNegDivHack = false,          // Gundam games messed up camera-view.
			XgKickHack = false,             // Erementar Gerad, adds more delay to VU XGkick instructions. Corrects the color of some graphics, but breaks Tri-ace games and others.
			IPUWaitHack = false,            // FFX FMV, makes GIF flush before doing IPU work. Fixes bad graphics overlay.
			EETimingHack = false,           // General purpose timing hack.
			SkipMPEGHack = false,           // Skips MPEG videos (Katamari and other games need this)
			OPHFlagHack = false,            // Bleach Blade Battlers
			DMABusyHack = false,            // Denies writes to the DMAC when it's busy. This is correct behaviour but bad timing can cause problems.
			VIFFIFOHack = false,            // Pretends to fill the non-existant VIF FIFO Buffer.
			VIF1StallHack = false,          // Like above, processes FIFO data before the stall is allowed (to make sure data goes over).
			GIFFIFOHack = false,            // Enabled the GIF FIFO (more correct but slower)
			FMVinSoftwareHack = false,      // Toggle in and out of software rendering when an FMV runs.
			GoemonTlbHack = false,          // Gomeon tlb miss hack. The game need to access unmapped virtual address. Instead to handle it as exception, tlb are preloaded at startup
			ScarfaceIbit = false,           // Scarface I bit hack. Needed to stop constant VU recompilation
			CrashTagTeamRacingIbit = false, // Crash Tag Team Racing I bit hack. Needed to stop constant VU recompilation
			VU0KickstartHack = false;       // Speed up VU0 at start of program to avoid some VU1 sync issues
		GamefixOptions();

		void load(std::shared_ptr<YamlFile> configSection);
		std::shared_ptr<YamlFile> save();

		GamefixOptions& DisableAll();

		void Set(const std::string& list, bool enabled = true);
		void Clear(const std::string& list) { Set(list, false); }

		bool Get(GamefixId id) const;
		void Set(GamefixId id, bool enabled = true);
		void Clear(GamefixId id) { Set(id, false); }

		bool operator==(const GamefixOptions& right) const;
		bool operator!=(const GamefixOptions& right) const;
	};

	// ------------------------------------------------------------------------
	struct SpeedhackOptions
	{
		bool
			fastCDVD = false,  // enables fast CDVD access
			IntcStat = false,  // tells Pcsx2 to fast-forward through intc_stat waits.
			WaitLoop = true,   // enables constant loop detection and fast-forwarding
			vuFlagHack = true, // microVU specific flag hack
			vuThread = true;   // Enable Threaded VU1

		s8 EECycleRate = 0; // EE cycle rate selector (1.0, 1.5, 2.0)
		u8 EECycleSkip = 0; // EE Cycle skip factor (0, 1, 2, or 3)

		SpeedhackOptions();

		void load(std::shared_ptr<YamlFile> configSection);
		std::shared_ptr<YamlFile> save();

		SpeedhackOptions& DisableAll();

		bool operator==(const SpeedhackOptions& right) const;
		bool operator!=(const SpeedhackOptions& right) const;
	};

	struct DebugOptions
	{
		bool ShowDebuggerOnStart = false;
		bool AlignMemoryWindowStart = true;

		u8 FontWidth = 8;
		u8 FontHeight = 12;
		u32 WindowWidth = 0;
		u32 WindowHeight = 0;
		u32 MemoryViewBytesPerRow = 16;

		DebugOptions();

		void load(std::shared_ptr<YamlFile> configSection);
		std::shared_ptr<YamlFile> save();

		bool operator==(const DebugOptions& right) const;
		bool operator!=(const DebugOptions& right) const;
	};

private:
	std::unique_ptr<YamlFile> config;

public:
	bool
		CdvdVerboseReads = false, // enables cdvd read activity verbosely dumped to the console
		CdvdDumpBlocks = false,   // enables cdvd block dumping
		CdvdShareWrite = false,   // allows the iso to be modified while it's loaded
		EnablePatches = true,     // enables patch detection and application
		EnableCheats = false,     // enables cheat detection and application
		EnableWideScreenPatches = false,
#ifndef DISABLE_RECORDING
		EnableRecordingTools = false,
#endif
		EnableIPC = false,
		// when enabled uses BOOT2 injection, skipping sony bios splashes
		UseBOOT2Injection = false,
		BackupSavestate = true,
		// enables simulated ejection of memory cards when loading savestates
		McdEnableEjection = true,
		McdFolderAutoManage = true,

		MultitapPort0_Enabled = false,
		MultitapPort1_Enabled = false,

		ConsoleToStdio = false,
		HostFs = false;

	fs::path BiosFilename;

	CpuOptions Cpu;
	GSOptions GS;
	SpeedhackOptions Speedhacks;
	GamefixOptions Gamefixes;
	ProfilerOptions Profiler;
	DebugOptions Debugger;
	RecompilerOptions Recompiler;
	TraceLogFilters Trace;

	Pcsx2Config();

	bool load();
	void save();

	bool MultitapEnabled(uint port) const;

	bool operator==(const Pcsx2Config& right) const;
	bool operator!=(const Pcsx2Config& right) const;
};

Pcsx2Config::GSOptions& SetGSConfig();
Pcsx2Config::RecompilerOptions& SetRecompilerConfig();
Pcsx2Config::GamefixOptions& SetGameFixConfig();
Pcsx2Config::TraceLogFilters& SetTraceConfig();


/////////////////////////////////////////////////////////////////////////////////////////
// Helper Macros for Reading Emu Configurations.
//

// ------------ CPU / Recompiler Options ---------------

#define THREAD_VU1 (g_Conf->emulator->Cpu.Recompiler.UseMicroVU1 && g_Conf->emulator->Speedhacks.vuThread)
#define CHECK_MICROVU0 (g_Conf->emulator->Cpu.Recompiler.UseMicroVU0)
#define CHECK_MICROVU1 (g_Conf->emulator->Cpu.Recompiler.UseMicroVU1)
#define CHECK_EEREC (g_Conf->emulator->Cpu.Recompiler.EnableEE && GetCpuProviders().IsRecAvailable_EE())
#define CHECK_CACHE (g_Conf->emulator->Cpu.Recompiler.EnableEECache)
#define CHECK_IOPREC (g_Conf->emulator->Cpu.Recompiler.EnableIOP && GetCpuProviders().IsRecAvailable_IOP())

//------------ SPECIAL GAME FIXES!!! ---------------
#define CHECK_VUADDSUBHACK (g_Conf->emulator->Gamefixes.VuAddSubHack) // Special Fix for Tri-ace games, they use an encryption algorithm that requires VU addi opcode to be bit-accurate.
#define CHECK_FPUCOMPAREHACK (g_Conf->emulator->Gamefixes.FpuCompareHack) // Special Fix for Digimon Rumble Arena 2, fixes spinning/hanging on intro-menu.
#define CHECK_FPUMULHACK (g_Conf->emulator->Gamefixes.FpuMulHack) // Special Fix for Tales of Destiny hangs.
#define CHECK_FPUNEGDIVHACK (g_Conf->emulator->Gamefixes.FpuNegDivHack) // Special Fix for Gundam games messed up camera-view.
#define CHECK_XGKICKHACK (g_Conf->emulator->Gamefixes.XgKickHack) // Special Fix for Erementar Gerad, adds more delay to VU XGkick instructions. Corrects the color of some graphics.
#define CHECK_IPUWAITHACK (g_Conf->emulator->Gamefixes.IPUWaitHack) // Special Fix For FFX
#define CHECK_EETIMINGHACK (g_Conf->emulator->Gamefixes.EETimingHack) // Fix all scheduled events to happen in 1 cycle.
#define CHECK_SKIPMPEGHACK (g_Conf->emulator->Gamefixes.SkipMPEGHack) // Finds sceMpegIsEnd pattern to tell the game the mpeg is finished (Katamari and a lot of games need this)
#define CHECK_OPHFLAGHACK (g_Conf->emulator->Gamefixes.OPHFlagHack) // Bleach Blade Battlers
#define CHECK_DMABUSYHACK (g_Conf->emulator->Gamefixes.DMABusyHack) // Denies writes to the DMAC when it's busy. This is correct behaviour but bad timing can cause problems.
#define CHECK_VIFFIFOHACK (g_Conf->emulator->Gamefixes.VIFFIFOHack) // Pretends to fill the non-existant VIF FIFO Buffer.
#define CHECK_VIF1STALLHACK (g_Conf->emulator->Gamefixes.VIF1StallHack) // Like above, processes FIFO data before the stall is allowed (to make sure data goes over).
#define CHECK_GIFFIFOHACK (g_Conf->emulator->Gamefixes.GIFFIFOHack) // Enabled the GIF FIFO (more correct but slower)

//------------ Advanced Options!!! ---------------
#define CHECK_VU_OVERFLOW (g_Conf->emulator->Cpu.Recompiler.vuOverflow)
#define CHECK_VU_EXTRA_OVERFLOW (g_Conf->emulator->Cpu.Recompiler.vuExtraOverflow) // If enabled, Operands are clamped before being used in the VU recs
#define CHECK_VU_SIGN_OVERFLOW (g_Conf->emulator->Cpu.Recompiler.vuSignOverflow)
#define CHECK_VU_UNDERFLOW (g_Conf->emulator->Cpu.Recompiler.vuUnderflow)
#define CHECK_VU_EXTRA_FLAGS 0 // Always disabled now // Sets correct flags in the sVU recs

#define CHECK_FPU_OVERFLOW (g_Conf->emulator->Cpu.Recompiler.fpuOverflow)
#define CHECK_FPU_EXTRA_OVERFLOW (g_Conf->emulator->Cpu.Recompiler.fpuExtraOverflow) // If enabled, Operands are checked for infinities before being used in the FPU recs
#define CHECK_FPU_EXTRA_FLAGS 1 // Always enabled now // Sets D/I flags on FPU instructions
#define CHECK_FPU_FULL (g_Conf->emulator->Cpu.Recompiler.fpuFullMode)

//------------ EE Recompiler defines - Comment to disable a recompiler ---------------

#define SHIFT_RECOMPILE // Speed majorly reduced if disabled
#define BRANCH_RECOMPILE // Speed extremely reduced if disabled - more then shift

// Disabling all the recompilers in this block is interesting, as it still runs at a reasonable rate.
// It also adds a few glitches. Really reminds me of the old Linux 64-bit version. --arcum42
#define ARITHMETICIMM_RECOMPILE
#define ARITHMETIC_RECOMPILE
#define MULTDIV_RECOMPILE
#define JUMP_RECOMPILE
#define LOADSTORE_RECOMPILE
#define MOVE_RECOMPILE
#define MMI_RECOMPILE
#define MMI0_RECOMPILE
#define MMI1_RECOMPILE
#define MMI2_RECOMPILE
#define MMI3_RECOMPILE
#define FPU_RECOMPILE
#define CP0_RECOMPILE
#define CP2_RECOMPILE

// You can't recompile ARITHMETICIMM without ARITHMETIC.
#ifndef ARITHMETIC_RECOMPILE
#undef ARITHMETICIMM_RECOMPILE
#endif

#define EE_CONST_PROP 1 // rec2 - enables constant propagation (faster)

// Change to 1 for console logs of SIF, GPU (PS1 mode) and MDEC (PS1 mode).
// These do spam a lot though!
#define PSX_EXTRALOGS 0

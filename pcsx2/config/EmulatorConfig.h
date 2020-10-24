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
			bool m_EnableAll, // Master Enable switch (if false, no logs at all)
				m_EnableDisasm,
				m_EnableRegisters,
				m_EnableEvents; // Enables logging of event-driven activity -- counters, DMAs, etc.

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
		bool EnableTraceLogFilters;

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
			EnableProfiler, // universal toggle for the profiler.
			RecBlocks_EE,   // Enables per-block profiling for the EE recompiler [unimplemented]
			RecBlocks_IOP,  // Enables per-block profiling for the IOP recompiler [unimplemented]
			RecBlocks_VU0,  // Enables per-block profiling for the VU0 recompiler [unimplemented]
			RecBlocks_VU1;  // Enables per-block profiling for the VU1 recompiler [unimplemented]

		void load(std::shared_ptr<YamlFile> configSection);
		std::shared_ptr<YamlFile> save();

		bool operator==(const ProfilerOptions& right) const;
		bool operator!=(const ProfilerOptions& right) const;
	};

	// ------------------------------------------------------------------------
	struct RecompilerOptions
	{
		bool
			EnableEE,
			EnableIOP,
			EnableVU0,
			EnableVU1;

		bool
			UseMicroVU0,
			UseMicroVU1;

		bool
			vuOverflow,
			vuExtraOverflow,
			vuSignOverflow,
			vuUnderflow;

		bool
			fpuOverflow,
			fpuExtraOverflow,
			fpuFullMode;

		bool
			StackFrameChecks,
			PreBlockCheckEE,
			PreBlockCheckIOP;
		bool
			EnableEECache;

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
		bool SynchronousMTGS;

		int VsyncQueueSize;

		bool FrameLimitEnable;
		bool FrameSkipEnable;
		VsyncMode VsyncEnable;

		int FramesToDraw; // number of consecutive frames (fields) to render
		int FramesToSkip; // number of consecutive frames (fields) to skip

		float LimitScalar;
		float FramerateNTSC;
		float FrameratePAL;

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
			VuAddSubHack,           // Tri-ace games, they use an encryption algorithm that requires VU ADDI opcode to be bit-accurate.
			FpuCompareHack,         // Digimon Rumble Arena 2, fixes spinning/hanging on intro-menu.
			FpuMulHack,             // Tales of Destiny hangs.
			FpuNegDivHack,          // Gundam games messed up camera-view.
			XgKickHack,             // Erementar Gerad, adds more delay to VU XGkick instructions. Corrects the color of some graphics, but breaks Tri-ace games and others.
			IPUWaitHack,            // FFX FMV, makes GIF flush before doing IPU work. Fixes bad graphics overlay.
			EETimingHack,           // General purpose timing hack.
			SkipMPEGHack,           // Skips MPEG videos (Katamari and other games need this)
			OPHFlagHack,            // Bleach Blade Battlers
			DMABusyHack,            // Denies writes to the DMAC when it's busy. This is correct behaviour but bad timing can cause problems.
			VIFFIFOHack,            // Pretends to fill the non-existant VIF FIFO Buffer.
			VIF1StallHack,          // Like above, processes FIFO data before the stall is allowed (to make sure data goes over).
			GIFFIFOHack,            // Enabled the GIF FIFO (more correct but slower)
			FMVinSoftwareHack,      // Toggle in and out of software rendering when an FMV runs.
			GoemonTlbHack,          // Gomeon tlb miss hack. The game need to access unmapped virtual address. Instead to handle it as exception, tlb are preloaded at startup
			ScarfaceIbit,           // Scarface I bit hack. Needed to stop constant VU recompilation
			CrashTagTeamRacingIbit, // Crash Tag Team Racing I bit hack. Needed to stop constant VU recompilation
			VU0KickstartHack;       // Speed up VU0 at start of program to avoid some VU1 sync issues
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
			fastCDVD,   // enables fast CDVD access
			IntcStat,   // tells Pcsx2 to fast-forward through intc_stat waits.
			WaitLoop,   // enables constant loop detection and fast-forwarding
			vuFlagHack, // microVU specific flag hack
			vuThread;   // Enable Threaded VU1

		s8 EECycleRate; // EE cycle rate selector (1.0, 1.5, 2.0)
		u8 EECycleSkip; // EE Cycle skip factor (0, 1, 2, or 3)

		SpeedhackOptions();

		void load(std::shared_ptr<YamlFile> configSection);
		std::shared_ptr<YamlFile> save();

		SpeedhackOptions& DisableAll();

		bool operator==(const SpeedhackOptions& right) const;
		bool operator!=(const SpeedhackOptions& right) const;
	};

	struct DebugOptions
	{
		bool ShowDebuggerOnStart;
		bool AlignMemoryWindowStart;

		u8 FontWidth;
		u8 FontHeight;
		u32 WindowWidth;
		u32 WindowHeight;
		u32 MemoryViewBytesPerRow;

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
		CdvdVerboseReads, // enables cdvd read activity verbosely dumped to the console
		CdvdDumpBlocks,   // enables cdvd block dumping
		CdvdShareWrite,   // allows the iso to be modified while it's loaded
		EnablePatches,    // enables patch detection and application
		EnableCheats,     // enables cheat detection and application
		EnableWideScreenPatches,
#ifndef DISABLE_RECORDING
		EnableRecordingTools,
#endif
		// when enabled uses BOOT2 injection, skipping sony bios splashes
		UseBOOT2Injection,
		BackupSavestate,
		// enables simulated ejection of memory cards when loading savestates
		McdEnableEjection,
		McdFolderAutoManage,

		MultitapPort0_Enabled,
		MultitapPort1_Enabled,

		ConsoleToStdio,
		HostFs;

	CpuOptions Cpu;
	GSOptions GS;
	SpeedhackOptions Speedhacks;
	GamefixOptions Gamefixes;
	ProfilerOptions Profiler;
	DebugOptions Debugger;

	RecompilerOptions Recompiler;

	TraceLogFilters Trace;

	fs::path BiosFilename;

	Pcsx2Config();

	bool EnableIPC;

	void loadFromFile(fs::path srcFile);
	void saveToFile(fs::path dstFile);

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

#define THREAD_VU1 (EmuConfig.Cpu.Recompiler.UseMicroVU1 && EmuConfig.Speedhacks.vuThread)
#define CHECK_MICROVU0 (EmuConfig.Cpu.Recompiler.UseMicroVU0)
#define CHECK_MICROVU1 (EmuConfig.Cpu.Recompiler.UseMicroVU1)
#define CHECK_EEREC (EmuConfig.Cpu.Recompiler.EnableEE && GetCpuProviders().IsRecAvailable_EE())
#define CHECK_CACHE (EmuConfig.Cpu.Recompiler.EnableEECache)
#define CHECK_IOPREC (EmuConfig.Cpu.Recompiler.EnableIOP && GetCpuProviders().IsRecAvailable_IOP())

//------------ SPECIAL GAME FIXES!!! ---------------
#define CHECK_VUADDSUBHACK (EmuConfig.Gamefixes.VuAddSubHack) // Special Fix for Tri-ace games, they use an encryption algorithm that requires VU addi opcode to be bit-accurate.
#define CHECK_FPUCOMPAREHACK (EmuConfig.Gamefixes.FpuCompareHack) // Special Fix for Digimon Rumble Arena 2, fixes spinning/hanging on intro-menu.
#define CHECK_FPUMULHACK (EmuConfig.Gamefixes.FpuMulHack) // Special Fix for Tales of Destiny hangs.
#define CHECK_FPUNEGDIVHACK (EmuConfig.Gamefixes.FpuNegDivHack) // Special Fix for Gundam games messed up camera-view.
#define CHECK_XGKICKHACK (EmuConfig.Gamefixes.XgKickHack) // Special Fix for Erementar Gerad, adds more delay to VU XGkick instructions. Corrects the color of some graphics.
#define CHECK_IPUWAITHACK (EmuConfig.Gamefixes.IPUWaitHack) // Special Fix For FFX
#define CHECK_EETIMINGHACK (EmuConfig.Gamefixes.EETimingHack) // Fix all scheduled events to happen in 1 cycle.
#define CHECK_SKIPMPEGHACK (EmuConfig.Gamefixes.SkipMPEGHack) // Finds sceMpegIsEnd pattern to tell the game the mpeg is finished (Katamari and a lot of games need this)
#define CHECK_OPHFLAGHACK (EmuConfig.Gamefixes.OPHFlagHack) // Bleach Blade Battlers
#define CHECK_DMABUSYHACK (EmuConfig.Gamefixes.DMABusyHack) // Denies writes to the DMAC when it's busy. This is correct behaviour but bad timing can cause problems.
#define CHECK_VIFFIFOHACK (EmuConfig.Gamefixes.VIFFIFOHack) // Pretends to fill the non-existant VIF FIFO Buffer.
#define CHECK_VIF1STALLHACK (EmuConfig.Gamefixes.VIF1StallHack) // Like above, processes FIFO data before the stall is allowed (to make sure data goes over).
#define CHECK_GIFFIFOHACK (EmuConfig.Gamefixes.GIFFIFOHack) // Enabled the GIF FIFO (more correct but slower)

//------------ Advanced Options!!! ---------------
#define CHECK_VU_OVERFLOW (EmuConfig.Cpu.Recompiler.vuOverflow)
#define CHECK_VU_EXTRA_OVERFLOW (EmuConfig.Cpu.Recompiler.vuExtraOverflow) // If enabled, Operands are clamped before being used in the VU recs
#define CHECK_VU_SIGN_OVERFLOW (EmuConfig.Cpu.Recompiler.vuSignOverflow)
#define CHECK_VU_UNDERFLOW (EmuConfig.Cpu.Recompiler.vuUnderflow)
#define CHECK_VU_EXTRA_FLAGS 0 // Always disabled now // Sets correct flags in the sVU recs

#define CHECK_FPU_OVERFLOW (EmuConfig.Cpu.Recompiler.fpuOverflow)
#define CHECK_FPU_EXTRA_OVERFLOW (EmuConfig.Cpu.Recompiler.fpuExtraOverflow) // If enabled, Operands are checked for infinities before being used in the FPU recs
#define CHECK_FPU_EXTRA_FLAGS 1 // Always enabled now // Sets D/I flags on FPU instructions
#define CHECK_FPU_FULL (EmuConfig.Cpu.Recompiler.fpuFullMode)

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

/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
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

#include <wx/fileconf.h>

#include "Config.h"
#include "GS.h"
#include "gui/GSFrame.h"

YAML::Node TraceLogFilters::LoadSave()
{
	//ScopedIniGroup path( json, L"TraceLog" );

	//IniEntry( Enabled );

	// Retaining backwards compat of the trace log enablers isn't really important, and
	// doing each one by hand would be murder.  So let's cheat and just save it as an int:

	//json.push_back(EE);
	//json.push_back(IOP);

	return YAML::Node();
}

Pcsx2Config::SpeedhackOptions::SpeedhackOptions()
{
	DisableAll();

	// Set recommended speedhacks to enabled by default. They'll still be off globally on resets.
	WaitLoop = true;
	IntcStat = true;
	vuFlagHack = true;
}

Pcsx2Config::SpeedhackOptions& Pcsx2Config::SpeedhackOptions::DisableAll()
{
	EECycleRate		= 0;
	EECycleSkip		= 0;

	return *this;
}

YAML::Node Pcsx2Config::SpeedhackOptions::LoadSave()
{
	YAML::Node speedHacks;

	speedHacks["EECycleRate"] = EECycleRate;
	speedHacks["EECycleSkip"] = EECycleSkip;
	speedHacks["fastCDVD"] = fastCDVD;
	speedHacks["IntcStat"] = IntcStat;
	speedHacks["WaitLoop"] = WaitLoop;
	speedHacks["vuFlagHack"] = vuFlagHack;
	speedHacks["vuThread"] = vuThread;

	return speedHacks;
}

void Pcsx2Config::SpeedhackOptions::Load(YAML::Node speedHacks)
{
    //EECycleRate = speedHacks["EECycleRate"].as<uint8_t>();
	//EECycleSkip = speedHacks["EECycleSkip"].as<uint8_t>();
	fastCDVD = speedHacks["fastCDVD"].as<bool>();
	IntcStat = speedHacks["IntcStat"].as<bool>();
	WaitLoop = speedHacks["WaitLoop"].as<bool>();
	vuFlagHack = speedHacks["vuFlagHack"].as<bool>();
	vuThread = speedHacks["vuThread"].as<bool>();
}

YAML::Node Pcsx2Config::ProfilerOptions::LoadSave()
{
	YAML::Node profiler;
	profiler["Enabled"] = Enabled;
	profiler["RecBlocks_EE"] = RecBlocks_EE;
	profiler["RecBlocks_IOP"] = RecBlocks_IOP;
	profiler["RecBlocks_VU0"] = RecBlocks_VU0;
	profiler["RecBlocks_VU1"] = RecBlocks_VU1;

    return profiler;
}

void Pcsx2Config::ProfilerOptions::Load(YAML::Node profiler)
{
	Enabled = profiler["Enabled"].as<bool>();
	RecBlocks_EE = profiler["RecBlocks_EE"].as<bool>();
	RecBlocks_IOP = profiler["RecBlocks_IOP"].as<bool>();
	RecBlocks_VU0 = profiler["RecBlocks_VU0"].as<bool>();
	RecBlocks_VU1 = profiler["RecBlocks_VU1"].as<bool>();
}


Pcsx2Config::RecompilerOptions::RecompilerOptions()
{
	StackFrameChecks	= false;
	PreBlockCheckEE		= false;

	// All recs are enabled by default.
	EnableEE	= true;
	EnableEECache = false;
	EnableIOP	= true;

	EnableVU0	= true;
	EnableVU1	= true;

	UseMicroVU0	= true;
	UseMicroVU1	= true;

	// vu and fpu clamping default to standard overflow.
	vuOverflow	= true;
	vuExtraOverflow = false;
	vuSignOverflow = false;
	vuUnderflow = false;

	fpuOverflow	= true;
	fpuExtraOverflow = false;
	fpuFullMode = false;


}

void Pcsx2Config::RecompilerOptions::ApplySanityCheck()
{
	bool fpuIsRight = true;

	if( fpuExtraOverflow )
		fpuIsRight = fpuOverflow;

	if( fpuFullMode )
		fpuIsRight = fpuOverflow && fpuExtraOverflow;

	if( !fpuIsRight )
	{
		// Values are wonky; assume the defaults.
		fpuOverflow		= RecompilerOptions().fpuOverflow;
		fpuExtraOverflow= RecompilerOptions().fpuExtraOverflow;
		fpuFullMode		= RecompilerOptions().fpuFullMode;
	}

	bool vuIsOk = true;

	if( vuExtraOverflow ) vuIsOk = vuIsOk && vuOverflow;
	if( vuSignOverflow ) vuIsOk = vuIsOk && vuExtraOverflow;

	if( !vuIsOk )
	{
		// Values are wonky; assume the defaults.
		vuOverflow		= RecompilerOptions().vuOverflow;
		vuExtraOverflow	= RecompilerOptions().vuExtraOverflow;
		vuSignOverflow	= RecompilerOptions().vuSignOverflow;
		vuUnderflow		= RecompilerOptions().vuUnderflow;
	}
}

YAML::Node Pcsx2Config::RecompilerOptions::LoadSave()
{
	YAML::Node recomp;

	recomp["EnableEE"] = EnableEE;
	recomp["EnableIOP"] = EnableIOP;
	recomp["EnableEECache"] = EnableEECache;
	recomp["EnableVU0"] = EnableVU0;
	recomp["EnableVU1"] = EnableVU1;

	recomp["vuOverflow"] = vuOverflow;
	recomp["vuExtraOverflow"] = vuExtraOverflow;
	recomp["vuSignOverflow"] = vuSignOverflow;
	recomp["vuUnderflow"] = vuUnderflow;

	recomp["fpuOverflow"] = fpuOverflow;
	recomp["fpuExtraOverflow"] = fpuExtraOverflow;
	recomp["fpuFullMode"] = fpuFullMode;

	recomp["StackFrameChecks"] = StackFrameChecks;
	recomp["PreBlockCheckEE"] = PreBlockCheckEE;
	recomp["PreBlockCheckIOP"] = PreBlockCheckIOP;

	return recomp;
}

void Pcsx2Config::RecompilerOptions::Load(YAML::Node recomp)
{
	EnableEE = recomp["EnableEE"].as<bool>();
	EnableIOP = recomp["EnableIOP"].as<bool>();
	EnableEECache = recomp["EnableEECache"].as<bool>();
	EnableVU0 = recomp["EnableVU0"].as<bool>();
	EnableVU1 = recomp["EnableVU1"].as<bool>();

	vuOverflow = recomp["vuOverflow"].as<bool>();
	vuExtraOverflow = recomp["vuExtraOverflow"].as<bool>();
	vuSignOverflow = recomp["vuSignOverflow"].as<bool>();
	vuUnderflow = recomp["vuUnderflow"].as<bool>();

	fpuOverflow = recomp["fpuOverflow"].as<bool>();
	fpuExtraOverflow = recomp["fpuExtraOverflow"].as<bool>();
	fpuFullMode = recomp["fpuFullMode"].as<bool>();

	StackFrameChecks = recomp["StackFrameChecks"].as<bool>();
    PreBlockCheckEE = recomp["PreBlockCheckEE"].as<bool>();
	PreBlockCheckIOP = recomp["PreBlockCheckIOP"].as<bool>();
}

Pcsx2Config::CpuOptions::CpuOptions()
{
	sseMXCSR.bitmask	= DEFAULT_sseMXCSR;
	sseVUMXCSR.bitmask	= DEFAULT_sseVUMXCSR;
}

void Pcsx2Config::CpuOptions::ApplySanityCheck()
{
	sseMXCSR.ClearExceptionFlags().DisableExceptions();
	sseVUMXCSR.ClearExceptionFlags().DisableExceptions();

	Recompiler.ApplySanityCheck();
}

YAML::Node Pcsx2Config::CpuOptions::LoadSave()
{
	YAML::Node cpu;

	cpu["VU.DenormalsAreZero"] =  sseVUMXCSR.DenormalsAreZero;
	cpu["VU.FlushToZero"] =  sseVUMXCSR.FlushToZero;
	cpu["VU.Roundmode"] =  sseVUMXCSR.RoundingControl;

	return cpu;
}

void Pcsx2Config::CpuOptions::Load(YAML::Node cpu)
{
	sseMXCSR.DenormalsAreZero = cpu["FPU.DenormalsAreZero"].as<uint32_t>(); 
	sseMXCSR.FlushToZero = cpu["FPU.FlushToZero"].as<uint32_t>();
	sseMXCSR.RoundingControl = cpu["FPU.Roundmode"].as<uint32_t>();
}

// Default GSOptions
Pcsx2Config::GSOptions::GSOptions()
{
	FrameLimitEnable		= true;
	FrameSkipEnable			= false;
	VsyncEnable				= VsyncMode::Off;

	SynchronousMTGS			= false;
	VsyncQueueSize			= 2;

	FramesToDraw			= 2;
	FramesToSkip			= 2;

	LimitScalar				= 1.0;
	FramerateNTSC			= 59.94;
	FrameratePAL			= 50.0;
}

YAML::Node Pcsx2Config::GSOptions::LoadSave()
{
	YAML::Node gs;

	gs["SynchronousMTGS"] = SynchronousMTGS;
	gs["VsyncQueueSize"] = VsyncQueueSize;

	gs["FrameLimitEnable"] = FrameLimitEnable;
	gs["FrameSkipEnable"] = FrameSkipEnable;
	//ini.EnumEntry( L"VsyncEnable", VsyncEnable, NULL, VsyncEnable );

	gs["LimitScalar"] = LimitScalar.ToFloat();
	gs["FramerateNTSC"] = FramerateNTSC.ToFloat();
	gs["FrameratePAL"] = FrameratePAL.ToFloat();

	gs["FramesToDraw"] = FramesToDraw;
	gs["FramesToSkip"] = FramesToSkip;

	return gs;
}

void Pcsx2Config::GSOptions::Load(YAML::Node gs)
{
	SynchronousMTGS = gs["SynchronousMTGS"].as<bool>();
	VsyncQueueSize = gs["VsyncQueueSize"].as<int>();
	FrameLimitEnable = gs["FrameLimitEnable"].as<bool>();
	FrameSkipEnable = gs["FrameSkipEnable"].as<bool>();
	//ini.EnumEntry( L"VsyncEnable", VsyncEnable, NULL, VsyncEnable );

	//gs["LimitScalar"] = LimitScalar.ToFloat();
	//gs["FramerateNTSC"] = FramerateNTSC.ToFloat();
	//gs["FrameratePAL"] = FrameratePAL.ToFloat();

	FramesToDraw = gs["FramesToDraw"].as<int>();
	FramesToSkip = gs["FramesToSkip"].as<int>();
}

int Pcsx2Config::GSOptions::GetVsync() const
{
	if (g_LimiterMode == Limit_Turbo || !FrameLimitEnable)
		return 0;

	// D3D only support a boolean state. OpenGL waits a number of vsync
	// interrupt (negative value for late vsync).
	switch (VsyncEnable) {
		case VsyncMode::Adaptive: return -1;
		case VsyncMode::Off: return 0;
		case VsyncMode::On: return 1;

		default: return 0;
	}
}

const wxChar *const tbl_GamefixNames[] =
{
	L"VuAddSub",
	L"FpuCompare",
	L"FpuMul",
	L"FpuNegDiv",
	L"XGKick",
	L"IpuWait",
	L"EETiming",
	L"SkipMpeg",
	L"OPHFlag",
	L"DMABusy",
	L"VIFFIFO",
	L"VIF1Stall",
	L"GIFFIFO",
	L"GoemonTlb",
	L"ScarfaceIbit",
	L"CrashTagTeamRacingIbit",
	L"VU0Kickstart"
};

const __fi wxChar* EnumToString( GamefixId id )
{
	return tbl_GamefixNames[id];
}

// all gamefixes are disabled by default.
Pcsx2Config::GamefixOptions::GamefixOptions()
{
	DisableAll();
}

Pcsx2Config::GamefixOptions& Pcsx2Config::GamefixOptions::DisableAll()
{
	return *this;
}

// Enables a full list of gamefixes.  The list can be either comma or pipe-delimited.
//   Example:  "XGKick,IpuWait"  or  "EEtiming,FpuCompare"
// If an unrecognized tag is encountered, a warning is printed to the console, but no error
// is generated.  This allows the system to function in the event that future versions of
// PCSX2 remove old hacks once they become obsolete.
void Pcsx2Config::GamefixOptions::Set( const std::string& list, bool enabled )
{
	//wxStringTokenizer izer( list, L",|", wxTOKEN_STRTOK );

	//while( izer.HasMoreTokens() )
	//{
		//wxString token( izer.GetNextToken() );

		//GamefixId i;
		//for (i=GamefixId_FIRST; i < pxEnumEnd; ++i)
		//{
		//	if( token.CmpNoCase( EnumToString(i) ) == 0 ) break;
		//}
		//if( i < pxEnumEnd ) Set( i );
	//}
}

void Pcsx2Config::GamefixOptions::Set( GamefixId id, bool enabled )
{
	EnumAssert( id );
	switch(id)
	{
		case Fix_VuAddSub:		VuAddSubHack		= enabled;	break;
		case Fix_FpuCompare:	FpuCompareHack		= enabled;	break;
		case Fix_FpuMultiply:	FpuMulHack			= enabled;	break;
		case Fix_FpuNegDiv:		FpuNegDivHack		= enabled;	break;
		case Fix_XGKick:		XgKickHack			= enabled;	break;
		case Fix_IpuWait:		IPUWaitHack			= enabled;	break;
		case Fix_EETiming:		EETimingHack		= enabled;	break;
		case Fix_SkipMpeg:		SkipMPEGHack		= enabled;	break;
		case Fix_OPHFlag:		OPHFlagHack			= enabled;  break;
		case Fix_DMABusy:		DMABusyHack			= enabled;  break;
		case Fix_VIFFIFO:		VIFFIFOHack			= enabled;  break;
		case Fix_VIF1Stall:		VIF1StallHack		= enabled;  break;
		case Fix_GIFFIFO:		GIFFIFOHack			= enabled;  break;
		case Fix_GoemonTlbMiss: GoemonTlbHack		= enabled;  break;
		case Fix_ScarfaceIbit:  ScarfaceIbit        = enabled;  break;
		case Fix_CrashTagTeamIbit: CrashTagTeamRacingIbit = enabled; break;
		case Fix_VU0Kickstart:	VU0KickstartHack	= enabled; break;
		jNO_DEFAULT;
	}
}

bool Pcsx2Config::GamefixOptions::Get( GamefixId id ) const
{
	EnumAssert( id );
	switch(id)
	{
		case Fix_VuAddSub:		return VuAddSubHack;
		case Fix_FpuCompare:	return FpuCompareHack;
		case Fix_FpuMultiply:	return FpuMulHack;
		case Fix_FpuNegDiv:		return FpuNegDivHack;
		case Fix_XGKick:		return XgKickHack;
		case Fix_IpuWait:		return IPUWaitHack;
		case Fix_EETiming:		return EETimingHack;
		case Fix_SkipMpeg:		return SkipMPEGHack;
		case Fix_OPHFlag:		return OPHFlagHack;
		case Fix_DMABusy:		return DMABusyHack;
		case Fix_VIFFIFO:		return VIFFIFOHack;
		case Fix_VIF1Stall:		return VIF1StallHack;
		case Fix_GIFFIFO:		return GIFFIFOHack;
		case Fix_GoemonTlbMiss: return GoemonTlbHack;
		case Fix_ScarfaceIbit:  return ScarfaceIbit;
		case Fix_CrashTagTeamIbit: return CrashTagTeamRacingIbit;
		case Fix_VU0Kickstart:	return VU0KickstartHack;
		jNO_DEFAULT;
	}
	return false;		// unreachable, but we still need to suppress warnings >_<
}

YAML::Node Pcsx2Config::GamefixOptions::LoadSave()
{

	YAML::Node gameFix;

	gameFix["VuAddSubHack"] = VuAddSubHack;
	gameFix["FpuCompareHack"] = FpuCompareHack;
	gameFix["FpuMulHack"] = FpuMulHack;
	gameFix["FpuNegDivHack"] = FpuNegDivHack;
	gameFix["XgKickHack"] = XgKickHack;
	gameFix["IPUWaitHack"] = IPUWaitHack;
	gameFix["EETimingHack"] = EETimingHack;
	gameFix["SkipMPEGHack"] = SkipMPEGHack;
	gameFix["OPHFlagHack"] = OPHFlagHack;
	gameFix["DMABusyHack"] = DMABusyHack;
	gameFix["VIFFIFOHack"] = VIFFIFOHack;
	gameFix["VIF1StallHack"] = VIF1StallHack;
	gameFix["GIFFIFOHack"] = GIFFIFOHack;
	gameFix["FMVinSoftwareHack"] = FMVinSoftwareHack;
	gameFix["GoemonTlbHack"] = GoemonTlbHack;
	gameFix["ScarfaceIbit"] = ScarfaceIbit;
    gameFix["CrashTagTeamRacingIbit"] = CrashTagTeamRacingIbit;

    return gameFix;
}

void Pcsx2Config::GamefixOptions::Load(YAML::Node gameFix)
{
	VuAddSubHack = gameFix["VuAddSubHack"].as<bool>();
	FpuCompareHack = gameFix["FpuCompareHack"].as<bool>();
	FpuMulHack = gameFix["FpuMulHack"].as<bool>();
	FpuNegDivHack = gameFix["FpuNegDivHack"].as<bool>();
	XgKickHack = gameFix["XgKickHack"].as<bool>();
	IPUWaitHack = gameFix["IPUWaitHack"].as<bool>();
	EETimingHack = gameFix["EETimingHack"].as<bool>();
	SkipMPEGHack = gameFix["SkipMPEGHack"].as<bool>();
	OPHFlagHack = gameFix["OPHFlagHack"].as<bool>();
	DMABusyHack = gameFix["DMABusyHack"].as<bool>();
	VIFFIFOHack = gameFix["VIFFIFOHack"].as<bool>();
	VIF1StallHack = gameFix["VIF1StallHack"].as<bool>();
	GIFFIFOHack = gameFix["GIFFIFOHack"].as<bool>();
	FMVinSoftwareHack = gameFix["FMVinSoftwareHack"].as<bool>();
	GoemonTlbHack = gameFix["GoemonTlbHack"].as<bool>();
	ScarfaceIbit = gameFix["ScarfaceIbit"].as<bool>();
    CrashTagTeamRacingIbit = gameFix["CrashTagTeamRacingIbit"].as<bool>();
}

Pcsx2Config::DebugOptions::DebugOptions()
{
	ShowDebuggerOnStart = false;
	AlignMemoryWindowStart = true;
	FontWidth = 8;
	FontHeight = 12;
	WindowWidth = 0;
	WindowHeight = 0;
	MemoryViewBytesPerRow = 16;
}


YAML::Node Pcsx2Config::DebugOptions::LoadSave()
{
	YAML::Node debugger;

	debugger["ShowDebuggerOnStart"] = ShowDebuggerOnStart;
	debugger["AlignMemoryWindowStart"] = AlignMemoryWindowStart;
	debugger["FontWidth"] = FontWidth;
	debugger["FontHeight"] = FontHeight;
	debugger["WindowWidth"] = WindowWidth;
	debugger["WindowHeight"] = WindowHeight;
	debugger["MemoryViewBytesPerRow"] = MemoryViewBytesPerRow;
	return debugger;
}

void Pcsx2Config::DebugOptions::Load(YAML::Node debugger)
{
	ShowDebuggerOnStart = debugger["ShowDebuggerOnStart"].as<bool>();
	AlignMemoryWindowStart = debugger["AlignMemoryWindowStart"].as<bool>();
	//FontWidth = debugger["FontWidth"].as<uint8_t>();
	//FontHeight = debugger["FontHeight"].as<uint8_t>();
	WindowWidth = debugger["WindowWidth"].as<uint32_t>();
	WindowHeight = debugger["WindowHeight"].as<uint32_t>();
	MemoryViewBytesPerRow = debugger["MemoryViewBytesPerRow"].as<uint32_t>();
}

Pcsx2Config::Pcsx2Config()
{
	// Set defaults for fresh installs / reset settings
	McdEnableEjection = true;
	McdFolderAutoManage = true;
	EnablePatches = true;
	BackupSavestate = true;
}

YAML::Node Pcsx2Config::LoadSave()
{
	YAML::Node core;

	core["CdvdVerboseReads"] = CdvdVerboseReads;
	core["CdvdDumpBlocks"] = CdvdDumpBlocks;
	core["CdvdShareWrite"] = CdvdShareWrite;
	core["EnablePatches"] = EnablePatches;
	core["EnableCheats"] = EnableCheats;
	core["EnableWideScreenPatches"] = EnableWideScreenPatches;
#ifndef DISABLE_RECORDING
	core["EnableRecordingTools"] = EnableRecordingTools;
#endif
	core["ConsoleToStdio"] = ConsoleToStdio;
	core["HostFs"] = HostFs;

	core["BackupSavestate"] = BackupSavestate;
	core["McdEnableEjection"] = McdEnableEjection;
	core["McdFolderAutoManage"] = McdFolderAutoManage;
	core["MultitapPort0_Enabled"] = MultitapPort0_Enabled;
	core["MultitapPort1_Enabled"] = MultitapPort1_Enabled;

	// Process various sub-components:

	YAML::Node toSave;

	toSave["Core"] = core;

	toSave["Recompiler"] = Recompiler.LoadSave();
	toSave["SpeedHacks"] = Speedhacks.LoadSave();
	toSave["Cpu"] = Cpu.LoadSave();
	toSave["GS"] = GS.LoadSave();
	toSave["GameFixes"] = Gamefixes.LoadSave();
	toSave["Profiler"] = Profiler.LoadSave();

	toSave["Debugger"] = Debugger.LoadSave();
	toSave["Trace"] = Trace.LoadSave();

	return toSave;
}


void Pcsx2Config::Load(YAML::Node& loader)
{

	std::string data;

	std::ostringstream os;
	os << loader;
	data = os.str();

	std::cout << "LOADER: " << data << std::endl;

	CdvdVerboseReads = loader["Core"]["CdvdVerboseReads"].as<bool>();
    CdvdDumpBlocks = loader["Core"]["CdvdDumpBlocks"].as<bool>();
	CdvdShareWrite = loader["Core"]["CdvdShareWrite"].as<bool>();
	EnablePatches = loader["Core"]["EnablePatches"].as<bool>();
	EnableCheats = loader["Core"]["EnableCheats"].as<bool>();
	EnableWideScreenPatches = loader["Core"]["EnableWideScreenPatches"].as<bool>();
#ifndef DISABLE_RECORDING
	EnableRecordingTools = loader["Core"]["EnableRecordingTools"].as<bool>();
#endif
	ConsoleToStdio = loader["Core"]["ConsoleToStdio"].as<bool>();
	HostFs = loader["Core"]["HostFs"].as<bool>();

	BackupSavestate = loader["Core"]["BackupSavestate"].as<bool>();
	McdEnableEjection = loader["Core"]["McdEnableEjection"].as<bool>();
	McdFolderAutoManage = loader["Core"]["McdFolderAutoManage"].as<bool>();
	MultitapPort0_Enabled = loader["Core"]["MultitapPort0_Enabled"].as<bool>();
	MultitapPort1_Enabled = loader["Core"]["MultitapPort1_Enabled"].as<bool>();
	
	Recompiler.Load(loader["Recompiler"]);
	Speedhacks.Load(loader["SpeedHacks"]);
	Gamefixes.Load(loader["GameFixes"]);
	Profiler.Load(loader["Profiler"]);
	Debugger.Load(loader["Debugger"]);
	//Cpu.Load(loader["Cpu"]);
	GS.Load(loader["GS"]);
}

bool Pcsx2Config::MultitapEnabled( uint port ) const
{
	pxAssert( port < 2 );
	return (port==0) ? MultitapPort0_Enabled : MultitapPort1_Enabled;
}

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

nlohmann::json TraceLogFilters::LoadSave()
{
	//ScopedIniGroup path( json, L"TraceLog" );

	//IniEntry( Enabled );

	// Retaining backwards compat of the trace log enablers isn't really important, and
	// doing each one by hand would be murder.  So let's cheat and just save it as an int:

	//json.push_back(EE);
	//json.push_back(IOP);

	return NULL;
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

nlohmann::json Pcsx2Config::SpeedhackOptions::LoadSave()
{
	nlohmann::json speedHacks;

	speedHacks["EECycleRate"] = EECycleRate;
	speedHacks["EECycleSkip"] = EECycleSkip;
	speedHacks["fastCDVD"] = fastCDVD;
	speedHacks["IntcStat"] = IntcStat;
	speedHacks["WaitLoop"] = WaitLoop;
	speedHacks["vuFlagHack"] = vuFlagHack;
	speedHacks["vuThread"] = vuThread;

	return speedHacks;
}

nlohmann::json Pcsx2Config::ProfilerOptions::LoadSave()
{
	nlohmann::json profiler;
	profiler["Enabled"] = Enabled;
	profiler["RecBlocks_EE"] = RecBlocks_EE;
	profiler["RecBlocks_IOP"] = RecBlocks_IOP;
	profiler["RecBlocks_VU0"] = RecBlocks_VU0;
	profiler["RecBlocks_VU1"] = RecBlocks_VU1;

    return profiler;
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

nlohmann::json Pcsx2Config::RecompilerOptions::LoadSave()
{
	nlohmann::json recomp;

	recomp["EnableEE"] = EnableEE;
	recomp["EnableIOP"] = EnableIOP;
	recomp["EnableEECache"] = EnableEECache;
	recomp["EnableVU0"] = EnableVU0;
	recomp["EnableVU1"] = EnableVU1;

	//recomp["UseMicroVU0"] = UseMicroVU0;
	//recomp["UseMicroVU1"] = UseMicroVU1;

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

nlohmann::json Pcsx2Config::CpuOptions::LoadSave()
{
	//ScopedIniGroup path( ini, L"CPU" );

	//IniBitBoolEx( sseMXCSR.DenormalsAreZero,	"FPU.DenormalsAreZero" );
	//IniBitBoolEx( sseMXCSR.FlushToZero,			"FPU.FlushToZero" );
	//IniBitfieldEx( sseMXCSR.RoundingControl,	"FPU.Roundmode" );

	//IniBitBoolEx( sseVUMXCSR.DenormalsAreZero,	"VU.DenormalsAreZero" );
	//IniBitBoolEx( sseVUMXCSR.FlushToZero,		"VU.FlushToZero" );
	//IniBitfieldEx( sseVUMXCSR.RoundingControl,	"VU.Roundmode" );

	//Recompiler.LoadSave();

	return NULL;
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

nlohmann::json Pcsx2Config::GSOptions::LoadSave()
{
	nlohmann::json gs;

	gs["SynchronousMTGS"] = SynchronousMTGS;
	gs["VsyncQueueSize"] = VsyncQueueSize;

	gs["FrameLimitEnable"] = FrameLimitEnable;
	gs["FrameSkipEnable"] = FrameSkipEnable;
	//ini.EnumEntry( L"VsyncEnable", VsyncEnable, NULL, VsyncEnable );

	//json["LimitScalar"] = LimitScalar;
	//json["FramerateNTSC"] = FramerateNTSC;
	//json["FrameratePAL"] = FrameratePAL;

	gs["FramesToDraw"] = FramesToDraw;
	gs["FramesToSkip"] = FramesToSkip;



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
	L"FMVinSoftware",
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
		case Fix_FMVinSoftware:	FMVinSoftwareHack	= enabled;  break;
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
		case Fix_FMVinSoftware:	return FMVinSoftwareHack;
		case Fix_GoemonTlbMiss: return GoemonTlbHack;
		case Fix_ScarfaceIbit:  return ScarfaceIbit;
		case Fix_CrashTagTeamIbit: return CrashTagTeamRacingIbit;
		case Fix_VU0Kickstart:	return VU0KickstartHack;
		jNO_DEFAULT;
	}
	return false;		// unreachable, but we still need to suppress warnings >_<
}

nlohmann::json Pcsx2Config::GamefixOptions::LoadSave()
{

	nlohmann::json gameFix;

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

nlohmann::json Pcsx2Config::DebugOptions::LoadSave()
{
	nlohmann::json debugger;

	debugger["ShowDebuggerOnStart"] = ShowDebuggerOnStart;
	debugger["AlignMemoryWindowStart"] = AlignMemoryWindowStart;
	debugger["FontWidth"] = FontWidth;
	debugger["FontHeight"] = FontHeight;
	debugger["WindowWidth"] = WindowWidth;
	debugger["WindowHeight"] = WindowHeight;
	debugger["MemoryViewBytesPerRow"] = MemoryViewBytesPerRow;
	return debugger;
}

Pcsx2Config::Pcsx2Config()
{
	// Set defaults for fresh installs / reset settings
	McdEnableEjection = true;
	McdFolderAutoManage = true;
	EnablePatches = true;
	BackupSavestate = true;
}

nlohmann::json Pcsx2Config::LoadSave()
{
	nlohmann::json core;

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

	auto array = nlohmann::json::array();

	array.push_back(core);

	array.push_back(Recompiler.LoadSave());
	array.push_back(Speedhacks.LoadSave());
	array.push_back(Cpu.LoadSave());
	array.push_back(GS.LoadSave());
	array.push_back(Gamefixes.LoadSave());
	array.push_back(Profiler.LoadSave());

	array.push_back(Debugger.LoadSave());
	array.push_back(Trace.LoadSave());

	return array;
}

bool Pcsx2Config::MultitapEnabled( uint port ) const
{
	pxAssert( port < 2 );
	return (port==0) ? MultitapPort0_Enabled : MultitapPort1_Enabled;
}

void Pcsx2Config::Load( nlohmann::json loader )
{
	//m_IsLoaded = true;

	//wxFileConfig cfg( srcfile );
	//IniLoader loader( cfg );
	//LoadSave( loader );
}

void Pcsx2Config::Save( std::string& dstfile )
{
	//if( !m_IsLoaded ) return;

	wxFileConfig cfg( dstfile );
	nlohmann::json saver;
	//LoadSave( saver );
}

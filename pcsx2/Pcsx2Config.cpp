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

void TraceLogFilters::LoadSave( nlohmann::json& json )
{
	//ScopedIniGroup path( json, L"TraceLog" );

	//IniEntry( Enabled );

	// Retaining backwards compat of the trace log enablers isn't really important, and
	// doing each one by hand would be murder.  So let's cheat and just save it as an int:

	//json.push_back(EE);
	//json.push_back(IOP);
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

void Pcsx2Config::SpeedhackOptions::LoadSave( nlohmann::json& json )
{
	//ScopedIniGroup path( ini, L"Speedhacks" );

	json["EECycleRate"] = EECycleRate;
	json["EECycleSkip"] = EECycleSkip;
	json["fastCDVD"] = fastCDVD;
	json["IntcStat"] = IntcStat;
	json["WaitLoop"] = WaitLoop;
	json["vuFlagHack"] = vuFlagHack;
	json["vuThread"] = vuThread;
}

void Pcsx2Config::ProfilerOptions::LoadSave( nlohmann::json& json )
{
	//ScopedIniGroup path( ini, L"Profiler" );

	json["Enabled"] = Enabled;
	json["RecBlocks_EE"] = RecBlocks_EE;
	json["RecBlocks_IOP"] = RecBlocks_IOP;
	json["RecBlocks_VU0"] = RecBlocks_VU0;
	json["RecBlocks_VU1"] = RecBlocks_VU1;
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

void Pcsx2Config::RecompilerOptions::LoadSave( nlohmann::json& json )
{
	//ScopedIniGroup path( ini, L"Recompiler" );

	json["EnableEE"] = EnableEE;
	json["EnableIOP"] = EnableIOP;
	json["EnableEECache"] = EnableEECache;
	json["EnableVU0"] = EnableVU0;
	json["EnableVU1"] = EnableVU1;

	json["UseMicroVU0"] = UseMicroVU0;
	json["UseMicroVU1"] = UseMicroVU1;

	json["vuOverflow"] = vuOverflow;
	json["vuExtraOverflow"] = vuExtraOverflow;
	json["vuSignOverflow"] = vuSignOverflow;
	json["vuUnderflow"] = vuUnderflow;

	json["fpuOverflow"] = fpuOverflow;
	json["fpuExtraOverflow"] = fpuExtraOverflow;
	json["fpuFullMode"] = fpuFullMode;

	json["StackFrameChecks"] = StackFrameChecks;
	json["PreBlockCheckEE"] = PreBlockCheckEE;
	json["PreBlockCheckIOP"] = PreBlockCheckIOP;
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

void Pcsx2Config::CpuOptions::LoadSave( nlohmann::json& json )
{
	//ScopedIniGroup path( ini, L"CPU" );

	//IniBitBoolEx( sseMXCSR.DenormalsAreZero,	"FPU.DenormalsAreZero" );
	//IniBitBoolEx( sseMXCSR.FlushToZero,			"FPU.FlushToZero" );
	//IniBitfieldEx( sseMXCSR.RoundingControl,	"FPU.Roundmode" );

	//IniBitBoolEx( sseVUMXCSR.DenormalsAreZero,	"VU.DenormalsAreZero" );
	//IniBitBoolEx( sseVUMXCSR.FlushToZero,		"VU.FlushToZero" );
	//IniBitfieldEx( sseVUMXCSR.RoundingControl,	"VU.Roundmode" );

	Recompiler.LoadSave( json );
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

void Pcsx2Config::GSOptions::LoadSave( nlohmann::json& json )
{
	//ScopedIniGroup path( ini, L"GS" );

	json["SynchronousMTGS"] = SynchronousMTGS;
	json["VsyncQueueSize"] = VsyncQueueSize;

	json["FrameLimitEnable"] = FrameLimitEnable;
	json["FrameSkipEnable"] = FrameSkipEnable;
	//ini.EnumEntry( L"VsyncEnable", VsyncEnable, NULL, VsyncEnable );

	//json["LimitScalar"] = LimitScalar;
	//json["FramerateNTSC"] = FramerateNTSC;
	//json["FrameratePAL"] = FrameratePAL;

	json["FramesToDraw"] = FramesToDraw;
	json["FramesToSkip"] = FramesToSkip;
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
    L"CrashTagTeamRacingIbit"
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
		jNO_DEFAULT;
	}
	return false;		// unreachable, but we still need to suppress warnings >_<
}

void Pcsx2Config::GamefixOptions::LoadSave( nlohmann::json& json )
{
	//ScopedIniGroup path( ini, L"Gamefixes" );

	json["VuAddSubHack"] = VuAddSubHack;
	json["FpuCompareHack"] = FpuCompareHack;
	json["FpuMulHack"] = FpuMulHack;
	json["FpuNegDivHack"] = FpuNegDivHack;
	json["XgKickHack"] = XgKickHack;
	json["IPUWaitHack"] = IPUWaitHack;
	json["EETimingHack"] = EETimingHack;
	json["SkipMPEGHack"] = SkipMPEGHack;
	json["OPHFlagHack"] = OPHFlagHack;
	json["DMABusyHack"] = DMABusyHack;
	json["VIFFIFOHack"] = VIFFIFOHack;
	json["VIF1StallHack"] = VIF1StallHack;
	json["GIFFIFOHack"] = GIFFIFOHack;
	json["FMVinSoftwareHack"] = FMVinSoftwareHack;
	json["GoemonTlbHack"] = GoemonTlbHack;
	json["ScarfaceIbit"] = ScarfaceIbit;
    json["CrashTagTeamRacingIbit"] = CrashTagTeamRacingIbit;
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

void Pcsx2Config::DebugOptions::LoadSave( nlohmann::json& json )
{
	//ScopedIniGroup path( ini, L"Debugger" );

	json["ShowDebuggerOnStart"] = ShowDebuggerOnStart;
	json["AlignMemoryWindowStart"] = AlignMemoryWindowStart;
	json["FontWidth"] = FontWidth;
	json["FontHeight"] = FontHeight;
	json["WindowWidth"] = WindowWidth;
	json["WindowHeight"] = WindowHeight;
	json["MemoryViewBytesPerRow"] = MemoryViewBytesPerRow;
}

Pcsx2Config::Pcsx2Config()
{
	// Set defaults for fresh installs / reset settings
	McdEnableEjection = true;
	McdFolderAutoManage = true;
	EnablePatches = true;
	BackupSavestate = true;
}

void Pcsx2Config::LoadSave( nlohmann::json& json )
{
	//ScopedIniGroup path( ini, L"EmuCore" );

	json["CdvdVerboseReads"] = CdvdVerboseReads;
	json["CdvdDumpBlocks"] = CdvdDumpBlocks;
	json["CdvdShareWrite"] = CdvdShareWrite;
	json["EnablePatches"] = EnablePatches;
	json["EnableCheats"] = EnableCheats;
	json["EnableWideScreenPatches"] = EnableWideScreenPatches;
#ifndef DISABLE_RECORDING
	json["EnableRecordingTools"] = EnableRecordingTools;
#endif
	json["ConsoleToStdio"] = ConsoleToStdio;
	json["HostFs"] = HostFs;

	json["BackupSavestate"] = BackupSavestate;
	json["McdEnableEjection"] = McdEnableEjection;
	json["McdFolderAutoManage"] = McdFolderAutoManage;
	json["MultitapPort0_Enabled"] = MultitapPort0_Enabled;
	json["MultitapPort1_Enabled"] = MultitapPort1_Enabled;

	// Process various sub-components:

	Speedhacks		.LoadSave( json );
	Cpu				.LoadSave( json );
	GS				.LoadSave( json );
	Gamefixes		.LoadSave( json );
	Profiler		.LoadSave( json );

	Debugger		.LoadSave( json );
	Trace			.LoadSave( json );

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
	LoadSave( loader );
}

void Pcsx2Config::Save( std::string& dstfile )
{
	//if( !m_IsLoaded ) return;

	wxFileConfig cfg( dstfile );
	nlohmann::json saver;
	LoadSave( saver );
}

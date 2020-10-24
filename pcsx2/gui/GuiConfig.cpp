#include "PrecompiledHeader.h"
#include "GuiConfig.h"
#include "App.h"

FolderUtils folderUtils;

namespace PathDefs
{
	// Specifies the root folder for the application install.
	// (currently it's the CWD, but in the future I intend to move all binaries to a "bin"
	// sub folder, in which case the approot will become "..") [- Air?]

	//The installer installs the folders which are relative to AppRoot (that's plugins/langs)
	//  relative to the exe folder, and not relative to cwd. So the exe should be default AppRoot. - avih
	fs::path AppRoot()
	{
		//AffinityAssert_AllowFrom_MainUI();

		if (InstallationMode == InstallMode_Registered)
		{
			//static const std::string cwdCache( (std::string)Path::Normalize(wxGetCwd()) );
			//return cwdCache;
		}
		else if (InstallationMode == InstallMode_Portable)

		if (InstallationMode == InstallMode_Registered || InstallationMode == InstallMode_Portable)
		{
			static const std::string appCache( (std::string)
			std::string(wxStandardPaths::Get().GetExecutablePath()));
			return appCache;
		}
		else
			pxFail( "Unimplemented user local folder mode encountered." );

		static const std::string dotFail(".");
		return dotFail;
	}

	// Specifies the main configuration folder.
	fs::path GetUserLocalDataDir()
	{
		fs::path temp = wxStandardPaths::Get().GetUserLocalDataDir().ToStdString();
    	return temp;
	}

	// Fetches the path location for user-consumable documents -- stuff users are likely to want to
	// share with other programs: screenshots, memory cards, and savestates.
	fs::path GetDocuments( DocsModeType mode )
	{
	switch( mode )
	{
#ifdef XDG_STD
			// Move all user data file into central configuration directory (XDG_CONFIG_DIR)
			case DocsFolder_User:	return GetUserLocalDataDir();
#else
			case DocsFolder_User:	return std::string((wxStandardPaths::Get().GetDocumentsDir(), pxGetAppName()));
#endif
			case DocsFolder_Custom: return CustomDocumentsFolder;

			jNO_DEFAULT
		}

		return GetDocuments(mode);
	}

	fs::path GetDocuments()
	{
		return GetDocuments( DocsFolderMode );
	}

	fs::path GetProgramDataDir()
	{
#ifndef GAMEINDEX_DIR_COMPILATION
		return AppRoot();
#else
	// Each linux distributions have his rules for path so we give them the possibility to
	// change it with compilation flags. -- Gregory
#define xGAMEINDEX_str(s) GAMEINDEX_DIR_str(s)
#define GAMEINDEX_DIR_str(s) #s
		return std::string( xGAMEINDEX_str(GAMEINDEX_DIR_COMPILATION) );
#endif
	}

	fs::path GetSnapshots()
	{
		return (GetDocuments() / "snapshots").make_preferred();
	}

	fs::path GetBios()
	{
		return (GetDocuments() / "bios").make_preferred();
	}

	fs::path GetCheats()
	{
		return (GetDocuments() / "cheats").make_preferred();
	}

	fs::path GetCheatsWS()
	{
		return (GetDocuments() / "cheats_ws").make_preferred();
	}

	fs::path GetDocs()
	{
		return (AppRoot().parent_path() / "docs").make_preferred();
	}

	fs::path GetSavestates()
	{
		return (GetDocuments() / "sstates").make_preferred();
	}

	fs::path GetMemoryCards()
	{
		return (GetDocuments() / "memcards").make_preferred();
	}

	fs::path GetPlugins()
	{
		// Each linux distributions have his rules for path so we give them the possibility to
		// change it with compilation flags. -- Gregory
#ifndef PLUGIN_DIR_COMPILATION
		return (AppRoot().parent_path() / "plugins").make_preferred();
#else
#define xPLUGIN_DIR_str(s) PLUGIN_DIR_str(s)
#define PLUGIN_DIR_str(s) #s
		return std::string( xPLUGIN_DIR_str(PLUGIN_DIR_COMPILATION) );
#endif
	}

	fs::path GetSettings()
	{
		fs::path docPath = GetDocuments();
		fs::path path = GetDocuments() / "settings";
		// make_preferred() is causing issues?
		return path;
	}

	fs::path GetLogs()
	{
		return (GetDocuments().parent_path() / "logs").make_preferred();
	}

	fs::path GetLangs()
	{
		return (AppRoot().parent_path() / "langs").make_preferred();
	}

	std::string Get( FoldersEnum_t folderidx )
	{
		switch( folderidx )
		{
			case FolderId_Plugins:		return GetPlugins();
			case FolderId_Settings:		return GetSettings();
			case FolderId_Bios:			return GetBios();
			case FolderId_Snapshots:	return GetSnapshots();
			case FolderId_Savestates:	return GetSavestates();
			case FolderId_MemoryCards:	return GetMemoryCards();
			case FolderId_Logs:			return GetLogs();
			case FolderId_Langs:		return GetLangs();
			case FolderId_Cheats:		return GetCheats();
			case FolderId_CheatsWS:		return GetCheatsWS();

			case FolderId_Documents:	return CustomDocumentsFolder;

			jNO_DEFAULT
		}
		return std::string(); // Aw hell naw
	}
};
std::string& FolderOptions::operator[]( FoldersEnum_t folderidx )
{
	switch( folderidx )
	{
		case FolderId_Plugins:		return PluginsFolder;
		case FolderId_Settings:		return SettingsFolder;
		case FolderId_Bios:			return Bios;
		case FolderId_Snapshots:	return Snapshots;
		case FolderId_Savestates:	return Savestates;
		case FolderId_MemoryCards:	return MemoryCards;
		case FolderId_Logs:			return Logs;
		case FolderId_Langs:		return Langs;
		case FolderId_Cheats:		return Cheats;
		case FolderId_CheatsWS:		return CheatsWS;

		case FolderId_Documents:	return CustomDocumentsFolder;

		jNO_DEFAULT
	}
	return PluginsFolder;		// unreachable, but suppresses warnings.
}

const std::string& FolderOptions::operator[]( FoldersEnum_t folderidx ) const
{
	return const_cast<FolderOptions*>( this )->operator[]( folderidx );
}

bool FolderOptions::IsDefault( FoldersEnum_t folderidx ) const
{
	switch( folderidx )
	{
		case FolderId_Plugins:		return UseDefaultPluginsFolder;
		case FolderId_Settings:		return UseDefaultSettingsFolder;
		case FolderId_Bios:			return UseDefaultBios;
		case FolderId_Snapshots:	return UseDefaultSnapshots;
		case FolderId_Savestates:	return UseDefaultSavestates;
		case FolderId_MemoryCards:	return UseDefaultMemoryCards;
		case FolderId_Logs:			return UseDefaultLogs;
		case FolderId_Langs:		return UseDefaultLangs;
		case FolderId_Cheats:		return UseDefaultCheats;
		case FolderId_CheatsWS:		return UseDefaultCheatsWS;

		case FolderId_Documents:	return false;

		jNO_DEFAULT
	}
	return false;
}

void FolderOptions::Set( FoldersEnum_t folderidx, const std::string& src, bool useDefault )
{
	switch( folderidx )
	{
		case FolderId_Plugins:
			PluginsFolder = src;
			UseDefaultPluginsFolder = useDefault;
		break;

		case FolderId_Settings:
			SettingsFolder = src;
			UseDefaultSettingsFolder = useDefault;
		break;

		case FolderId_Bios:
			Bios = src;
			UseDefaultBios = useDefault;
		break;

		case FolderId_Snapshots:
			Snapshots = src;
			UseDefaultSnapshots = useDefault;
		break;

		case FolderId_Savestates:
			Savestates = src;
			UseDefaultSavestates = useDefault;
		break;

		case FolderId_MemoryCards:
			MemoryCards = src;
			UseDefaultMemoryCards = useDefault;
		break;

		case FolderId_Logs:
			Logs = src;
			UseDefaultLogs = useDefault;
		break;

		case FolderId_Langs:
			Langs = src;
			UseDefaultLangs = useDefault;
		break;

		case FolderId_Documents:
			CustomDocumentsFolder = src;
		break;

		case FolderId_Cheats:
			Cheats = src;
			UseDefaultCheats = useDefault;
		break;

		case FolderId_CheatsWS:
			CheatsWS = src;
			UseDefaultCheatsWS = useDefault;
		break;

		jNO_DEFAULT
	}
}

// --------------------------------------------------------------------------------------
//  Default Filenames
// --------------------------------------------------------------------------------------
namespace FilenameDefs
{
	std::string GetUiConfig()
	{
		return pxGetAppName().ToStdString() + "_ui.yaml";
	}

	std::string GetUiKeysConfig()
	{
		return pxGetAppName().ToStdString() + "_keys.yaml";
	}

	std::string GetVmConfig()
	{
		return pxGetAppName().ToStdString() + "_vm.yaml";
	}

	std::string GetUsermodeConfig()
	{
		return ("usermode.yaml");
	}

	const std::string& Memcard(uint port, uint slot)
	{
		static const std::string retval[2][4] =
			{
				{
					std::string("Mcd001.ps2"),
					std::string("Mcd003.ps2"),
					std::string("Mcd005.ps2"),
					std::string("Mcd007.ps2"),
				},
				{
					std::string("Mcd002.ps2"),
					std::string("Mcd004.ps2"),
					std::string("Mcd006.ps2"),
					std::string("Mcd008.ps2"),
				}};

		IndexBoundsAssumeDev(L"FilenameDefs::Memcard", port, 2);
		IndexBoundsAssumeDev(L"FilenameDefs::Memcard", slot, 4);

		return retval[port][slot];
	}
}; // namespace FilenameDefs


std::string GuiConfig::FullpathTo( PluginsEnum_t pluginidx ) const
{
	return Path::Combine(PluginsFolder,  BaseFilenames[pluginidx] );
}

// returns true if the filenames are quite absolutely the equivalent.  Works for all
// types of filenames, relative and absolute.  Very important that you use this function
// rather than any other type of more direct string comparison!
bool GuiConfig::FullpathMatchTest( PluginsEnum_t pluginId, const wxString& cmpto )
{
	// Implementation note: wxFileName automatically normalizes things as needed in it's
	// equality comparison implementations, so we can do a simple comparison as follows:

	return wxFileName(cmpto).SameAs((wxFileName)FullpathTo(pluginId) );
}

static std::string GetResolvedFolder(FoldersEnum_t id)
{
	return g_Conf->Folders.IsDefault(id) ? PathDefs::Get(id) : g_Conf->Folders[id];
}

fs::path GetLogFolder()
{
	return GetResolvedFolder(FolderId_Logs);
}

fs::path GetCheatsFolder()
{
	return GetResolvedFolder(FolderId_Cheats);
}

fs::path GetCheatsWsFolder()
{
	return GetResolvedFolder(FolderId_CheatsWS);
}

fs::path GetSettingsFolder()
{
	if( !wxGetApp().Overrides.SettingsFolder.empty() )
		return wxGetApp().Overrides.SettingsFolder;

	return UseDefaultSettingsFolder ? PathDefs::GetSettings().string() : SettingsFolder;
}

fs::path GetVmSettingsFilename()
{
    fs::path fname( !wxGetApp().Overrides.VmSettingsFile.empty() ? wxGetApp().Overrides.VmSettingsFile : FilenameDefs::GetVmConfig() );
	std::cout << "Path: " << Path::Combine(GetSettingsFolder(), fname) << std::endl;
    return Path::Combine(GetSettingsFolder(), fname);
}

fs::path GetUiSettingsFilename()
{
	fs::path fname( FilenameDefs::GetUiConfig() );
	return (GetSettingsFolder() / fname).make_preferred();
}

fs::path GetUiKeysFilename()
{
	fs::path fname( FilenameDefs::GetUiKeysConfig() );
	return (GetSettingsFolder() / fname).make_preferred();
}

std::string GuiConfig::FullpathToBios() const				
{ 
	return Path::Combine(Folders.Bios, BaseFilenames.Bios.ToStdString());
}

std::string GuiConfig::FullpathToMcd( uint slot ) const
{
	return Path::Combine( Folders.MemoryCards, Mcd[slot].Filename.GetFullName().ToStdString() );
}

// ------------------------------------------------------------------------
bool GuiConfig::SaveMemcards(wxConfigBase* conf)
{
	
	for( uint slot=0; slot<2; ++slot )
	{
		//memcards[fmt::format("Slot{}u_Enable", slot)] = Mcd[slot].Enabled;
		//memcards[fmt::format("Slot{}u_Filename", slot)] = Mcd[slot].Filename.GetFullName().ToStdString();
	}

	for( uint slot=2; slot<8; ++slot )
	{
		//int mtport = FileMcd_GetMtapPort(slot)+1;
		//int mtslot = FileMcd_GetMtapSlot(slot)+1;

		//memcards[fmt::format("Multitap%u_Slot%u_Enable", mtport, mtslot)] = Mcd[slot].Enabled;
		//memcards[fmt::format("Multitap%u_Slot%u_Filename", mtport, mtslot)] = Mcd[slot].Filename.GetFullName().ToStdString();
	}


	return true;
}

bool GuiConfig::SaveRootItems(wxConfigBase* base)
{
	// TODO - YAML

	//YAML::Node yaml;

	//yaml["RecentIsoCount"] = RecentIsoCount;
	//yaml["Listbook_ImageSize"] = Listbook_ImageSize;
	//yaml["Toolbar_ImageSize"] = Toolbar_ImageSize;
	//yaml["Toolbar_ShowLabels"] = Toolbar_ShowLabels;

	//std::string res = CurrentIso;
	//yaml["CurrentIso"] = res; // TODO - missing the allow relative flag
	//CurrentIso = res;

	//yaml["CurrentBlockdump"] = CurrentBlockdump;
	//yaml["CurrentELF"] = CurrentELF;
	//yaml["CurrentIRX"] = CurrentIRX;

	//yaml["EnableSpeedHacks"] = EnableSpeedHacks;
	//yaml["EnableGameFixes"] = EnableGameFixes;
	//yaml["EnableFastBoot"] = EnableFastBoot;

	//yaml["EnablePresets"] = EnablePresets;
	//yaml["PresetIndex"] = PresetIndex;
	//yaml["AskOnBoot"] =  AskOnBoot;

	//#ifdef __WXMSW__
	////IniEntry( McdCompressNTFS );
	//#endif

	//// TODO - these are not basic types at all
	////yaml["CdvdSource"] = (CdvdSource, CDVD_SourceLabels, CdvdSource );
	return true;
}

// ------------------------------------------------------------------------
bool GuiConfig::Save(wxConfigBase* conf)
{
	SaveRootItems(conf);
	SaveMemcards(conf);

	// Process various sub-components:

	Folders.Save(conf);
	Framerate.Save(conf);
	//json.push_back(Templates.LoadSave());
	return true;
}

// ------------------------------------------------------------------------
void FolderOptions::ApplyDefaults()
{
	if( UseDefaultBios )		Bios		  = PathDefs::GetBios();
	if( UseDefaultSnapshots )	Snapshots	  = PathDefs::GetSnapshots();
	if( UseDefaultSavestates )	Savestates	  = PathDefs::GetSavestates();
	if( UseDefaultMemoryCards )	MemoryCards	  = PathDefs::GetMemoryCards();
	if( UseDefaultLogs )		Logs		  = PathDefs::GetLogs();
	if( UseDefaultLangs )		Langs		  = PathDefs::GetLangs();
	if( UseDefaultPluginsFolder)PluginsFolder = PathDefs::GetPlugins();
	if( UseDefaultCheats )      Cheats		  = PathDefs::GetCheats();
	if( UseDefaultCheatsWS )    CheatsWS	  = PathDefs::GetCheatsWS();
}

// ------------------------------------------------------------------------
    FolderOptions::FolderOptions()
	: Bios			( PathDefs::GetBios() )
	, Snapshots		( PathDefs::GetSnapshots() )
	, Savestates	( PathDefs::GetSavestates() )
	, MemoryCards	( PathDefs::GetMemoryCards() )
	, Langs			( PathDefs::GetLangs() )
	, Logs			( PathDefs::GetLogs() )
	, Cheats		( PathDefs::GetCheats() )
	, CheatsWS      ( PathDefs::GetCheatsWS() )

	, RunIso	( PathDefs::GetDocuments() )			// raw default is always the Documents folder.
	, RunELF	( PathDefs::GetDocuments() )			// raw default is always the Documents folder.
	, RunDisc	( PathDefs::GetDocuments() )
{
	//bitset = 0xffffffff;
}

bool FolderOptions::Save(wxConfigBase* conf) // conf write = write to config file
{

	//when saving in portable mode, we save relative paths if possible
	 //  --> on load, these relative paths will be expanded relative to the exe folder.
	bool rel = true;


	if (conf->Write("UseDefaultBios", UseDefaultBios) &&
	conf->Write("UseDefaultSavestates", UseDefaultSavestates) &&
	conf->Write("UseDefaultMemoryCards", UseDefaultMemoryCards) &&
	conf->Write("UseDefaultLogs", UseDefaultLogs) &&
	conf->Write("UseDefaultLangs", UseDefaultLangs) &&
	conf->Write("UseDefaultPluginsFolder", UseDefaultPluginsFolder) &&
	conf->Write("UseDefaultCheats", UseDefaultCheats) &&
	conf->Write("UseDefaultCheatsWS", UseDefaultCheatsWS) &&

	conf->Write(Bios, rel) &&
	conf->Write(Snapshots, rel) &&
	conf->Write(Savestates, rel) &&
	conf->Write(MemoryCards, rel) &&
	conf->Write(Logs, rel) &&
	conf->Write(Langs, rel) &&
	conf->Write(Cheats, rel) &&
	conf->Write(CheatsWS, rel) &&
	conf->Write(PluginsFolder, wxString(Path::Combine(InstallFolder, "plugins"))) &&

	conf->Write(RunIso.wstring(), rel) &&
	conf->Write(RunELF.wstring(), rel))

{
	return true;
}
//		ApplyDefaults();

		for( int i=0; i<FolderId_COUNT; ++i )
			operator[]( (FoldersEnum_t)i );
}

#ifndef DISABLE_RECORDING
InputRecordingOptions::InputRecordingOptions()
	: VirtualPadPosition(wxDefaultPosition)
{
}

bool InputRecordingOptions::Save(wxConfigBase* conf)
{
	if(conf->Write("VirtualPadPositionX",VirtualPadPosition.x) &&
		conf->Write("VirtualPadPositionY", VirtualPadPosition.y))
	{
		return true;
	}

	else
	{
		return false;
	}

}
#endif

// ----------------------------------------------------------------------------
FramerateOptions::FramerateOptions()
{
	NominalScalar			= 1.0;
	TurboScalar				= 2.0;
	SlomoScalar				= 0.50;

	SkipOnLimit				= false;
	SkipOnTurbo				= false;
}


bool FramerateOptions::Save(wxConfigBase* conf)
{
	//ScopedIniVurboScalar );
	if (conf->Write("SlomoScalar", SlomoScalar))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void FramerateOptions::SanityCheck()
{
	// Ensure Conformation of various options...

	//NominalScalar	.ConfineTo( 0.05, 10.0 );
	//TurboScalar		.ConfineTo( 0.05, 10.0 );
	//SlomoScalar		.ConfineTo( 0.05, 10.0 );
}


UiTemplateOptions::UiTemplateOptions()
{
	LimiterUnlimited	= "Max";
	LimiterTurbo		= "Turbo";
	LimiterSlowmo		= "Slowmo";
	LimiterNormal		= "Normal";
	OutputFrame			= "Frame";
	OutputField			= "Field";
	OutputProgressive	= "Progressive";
	OutputInterlaced	= "Interlaced";
	Paused				= "<PAUSED> ";
	TitleTemplate		= "Slot: ${slot} | Speed: ${speed} (${vfps}) | ${videomode} | Limiter: ${limiter} | ${gsdx} | ${omodei} | ${cpuusage}";
#ifndef DISABLE_RECORDING
	RecordingTemplate	= "Slot: ${slot} | Frame: ${frame}/${maxFrame} | Rec. Mode: ${mode} | Speed: ${speed} (${vfps}) | Limiter: ${limiter}";
#endif
}

/*nlohmann::json AppConfig::UiTemplateOptions::LoadSave();
{
	nlohmann::json ui;

	ui["LimiterUnlimited"] = LimiterUnlimited;
	ui["LimiterTurbo"] = LimiterTurbo;
	ui["LimiterSlowmo"] = LimiterSlowmo;
	ui["LimiterNormal"] = LimiterNormal;
	ui["OutputFrame"] = OutputFrame;
	ui["OutputField"] = OutputField;
	ui["OutputProgressive"] = OutputProgressive;
	ui["OutputInterlaced"] = OutputInterlaced;
	ui["Paused"] = Paused;
	ui["TitleTemplate"] = TitleTemplate;
#ifndef DISABLE_RECORDING
	ui["RecordingTemplate"] = RecordingTemplate;
#endif

return ui;
}*/

int GuiConfig::GetMaxPresetIndex()
{
	return 5;
}

bool GuiConfig::isOkGetPresetTextAndColor( int n, std::string& label, wxColor& c )
{
	const std::string presetNamesAndColors[][2] =
	{
		{ ("Safest (No hacks)"), "Blue" },
		{ ("Safe (Default)"),	"Dark Green" },
		{ ("Balanced"),			"Forest Green" },
		{ ("Aggressive"),		"Orange" },
		{ ("Very Aggressive"),	"Red"},
		{ ("Mostly Harmful"),	"Purple" }
	};
	if( n<0 || n>GetMaxPresetIndex() )
		return false;

	label = wxsFormat(L"%d - ", n+1) + presetNamesAndColors[n][0];
	c	  = wxColor(presetNamesAndColors[n][1]);

    return true;
}










ConsoleLogOptions::ConsoleLogOptions()
	: DisplayPosition{100, 100}
	, DisplaySize{680, 560}
	, Theme("Default")
{
	Visible = true;
	AutoDock = true;
	FontSize = 8;
}

bool ConsoleLogOptions::Save(wxConfigBase* conf)
{
	conf->Write("Theme", Theme);
	conf->Write("FontSize", FontSize);
	conf->Write("IsVisible", Visible);
	conf->Write("Autodock", AutoDock);
	conf->Write("DisplaySizeX", DisplaySize.x);
	conf->Write("DisplaySizeY", DisplaySize.y);
	conf->Write("DisplayPositionX", DisplayPosition.x);
	conf->Write("DisplayPositionY", DisplayPosition.y);
	return true;
}

void ConsoleLogOptions::Load(wxConfigBase* conf)
{
	conf->Read("Theme", Theme);
	conf->Read("FontSize", FontSize);
	conf->Read("IsVisible", Visible);
	conf->Read("Autodock", AutoDock);
	conf->ReadObject("DisplaySizeX", DisplaySize.x);
	conf->ReadObject("DisplaySizeY", DisplaySize.y);
	conf->ReadObject("DisplayPositionX", DisplayPosition.x);
	conf->ReadObject("DisplayPositionY", DisplayPosition.y);
}

GSWindowOptions::GSWindowOptions()
{
	CloseOnEsc = true;
	DefaultToFullscreen = false;
	AlwaysHideMouse = false;
	DisableResizeBorders = false;
	DisableScreenSaver = true;

	AspectRatio = AspectRatio_4_3;
	FMVAspectRatioSwitch = FMV_AspectRatio_Switch_Off;
	Zoom = 100;
	StretchY = 100;
	OffsetX = 0;
	OffsetY = 0;

	WindowSize = wxSize(640, 480);
	WindowPos = wxDefaultPosition;
	IsMaximized = false;
	IsFullscreen = false;
	EnableVsyncWindowFlag = false;

	IsToggleFullscreenOnDoubleClick = true;
}

void GSWindowOptions::Save(wxConfigBase* conf)
{
	conf->Write("Zoom", Zoom);
	conf->Write("OffsetX", OffsetX);
	conf->Write("OffsetY", OffsetY);
	conf->Write("StretchY", StretchY);
	conf->Write("WindowPosX", WindowPos.x);
	conf->Write("WindowPosY", WindowPos.y);
	conf->Write("WindowSizeX", WindowSize.x);
	conf->Write("WindowSizeY", WindowSize.y);
	conf->Write("CloseOnEsc", CloseOnEsc);
	conf->Write("AspectRatio", (int)AspectRatio);
	conf->Write("IsMaximized", IsMaximized);
	conf->Write("IsFullscreen", IsFullscreen);
	conf->Write("AlwaysHideMouse", AlwaysHideMouse);
	conf->Write("DisableScreenSaver", DisableScreenSaver);
	conf->Write("DefaultToFullScreen", DefaultToFullscreen);
	conf->Write("DisableResizeBorders", DisableResizeBorders);
	conf->Write("FMVAspectRatioSwitch", (int)FMVAspectRatioSwitch);
	conf->Write("EnableVsyncWindowFlag", EnableVsyncWindowFlag);
	conf->Write("IsToggleFullscreenOnDoubleClick", IsToggleFullscreenOnDoubleClick);
}

void GSWindowOptions::Load(wxConfigBase* conf)
{
	conf->Read("Zoom", Zoom);
	conf->Read("OffsetX", OffsetX);
	conf->Read("OffsetY", OffsetY);
	conf->Read("StretchY", StretchY);
	conf->ReadObject("WindowPosX", WindowPos.x);
	conf->ReadObject("WindowPosY", WindowPos.y);
	conf->ReadObject("WindowSizeX", WindowSize.x);
	conf->ReadObject("WindowSizeY", WindowSize.y);
	conf->Read("CloseOnEsc", CloseOnEsc);
	conf->Read("AspectRatio", (int)AspectRatio);
	conf->Read("IsMaximized", IsMaximized);
	conf->Read("IsFullscreen", IsFullscreen);
	conf->Read("AlwaysHideMouse", AlwaysHideMouse);
	conf->Read("DisableScreenSaver", DisableScreenSaver);
	conf->Read("DefaultToFullScreen", DefaultToFullscreen);
	conf->Read("DisableResizeBorders", DisableResizeBorders);
	conf->Read("FMVAspectRatioSwitch", (int)FMVAspectRatioSwitch);
	conf->Read("EnableVsyncWindowFlag", EnableVsyncWindowFlag);
	conf->Read("IsToggleFullscreenOnDoubleClick", IsToggleFullscreenOnDoubleClick);
}

void GSWindowOptions::SanityCheck()
{
	// Ensure Conformation of various options...

	WindowSize.x = std::max(WindowSize.x, 8);
	WindowSize.x = std::min(WindowSize.x, wxGetDisplayArea().GetWidth() - 16);

	WindowSize.y = std::max(WindowSize.y, 8);
	WindowSize.y = std::min(WindowSize.y, wxGetDisplayArea().GetHeight() - 48);

	// Make sure the upper left corner of the window is visible enought o grab and
	// move into view:
	if (!wxGetDisplayArea().Contains(wxRect(WindowPos, wxSize(48, 48))))
		WindowPos = wxDefaultPosition;

	if ((uint)AspectRatio >= (uint)AspectRatio_MaxCount)
		AspectRatio = AspectRatio_4_3;
}


// ------------------------------------------------------------------------
const std::string& FilenameOptions::operator[](PluginsEnum_t pluginidx) const
{
	IndexBoundsAssumeDev("Filename[Plugin]", pluginidx, PluginId_Count);
	return Plugins[pluginidx].ToStdString();
}

void FilenameOptions::Save(wxConfigBase* conf)
{

	static const std::string pc("Please Configure");

	//when saving in portable mode, we just save the non-full-path filename
	//  --> on load they'll be initialized with default (relative) paths (works both for plugins and bios)
	//note: this will break if converting from install to portable, and custom folders are used. We can live with that.
	bool needRelativeName = false; //ini.IsSaving() && IsPortable();

	for (int i = 0; i < PluginId_Count; ++i)
	{
		wxString pluginShortName = static_cast<std::string>(tbl_PluginInfo[i].GetShortname());
		if (needRelativeName)
		{
			wxFileName plugin_filename = wxFileName(Plugins[i]);
			conf->Write(pluginShortName, plugin_filename.GetFullPath());
		} else
		conf->Write(pluginShortName, wxString(Plugins[i]));
	}

	if (needRelativeName)
	{
		wxString bios_filename = Bios;
		conf->Write(L"BIOS", bios_filename);
	}
	else
		conf->Write(L"BIOS", wxString(pc));

}

void FilenameOptions::Load(wxConfigBase* conf)
{
	conf->Read("BIOS", Bios);
}


// Apply one of several (currently 6) configuration subsets.
// The scope of the subset which each preset controlls is hardcoded here.
// Use ignoreMTVU to avoid updating the MTVU field.
// Main purpose is for the preset enforcement at launch, to avoid overwriting a user's setting.
bool GuiConfig::IsOkApplyPreset(int n, bool ignoreMTVU)
{
	if (n < 0 || n > GetMaxPresetIndex() )
	{
		Console.WriteLn("DEV Warning: ApplyPreset(%d): index out of range, Aborting.", n);
		return false;
	}

	//Console.WriteLn("Applying Preset %d ...", n);

	//Have some original and default values at hand to be used later.
	FramerateOptions	    original_Framerate = Framerate;
	GuiConfig				default_AppConfig;
	Pcsx2Config				default_Pcsx2Config;

	//  NOTE:	Because the system currently only supports passing of an entire AppConfig to the GUI panels/menus to apply/reflect,
	//			the GUI entities should be aware of the settings which the presets control, such that when presets are used:
	//			1. The panels/entities should prevent manual modifications (by graying out) of settings which the presets control.
	//			2. The panels should not apply values which the presets don't control if the value is initiated by a preset.
	//			Currently controlled by the presets:
	//			- AppConfig:	Framerate (except turbo/slowmo factors), EnableSpeedHacks, EnableGameFixes.
	//			- EmuOptions:	Cpu, Gamefixes, SpeedHacks (except mtvu), EnablePatches, GS (except for FrameLimitEnable and VsyncEnable).
	//
	//			This essentially currently covers all the options on all the panels except for framelimiter which isn't
	//			controlled by the presets, and the entire GSWindow panel which also isn't controlled by presets
	//
	//			So, if changing the scope of the presets (making them affect more or less values), the relevant GUI entities
	//			should me modified to support it.

	//Force some settings as a (current) base for all presets.

	Framerate			= default_AppConfig.Framerate;
	Framerate.SlomoScalar = original_Framerate.SlomoScalar;
	Framerate.TurboScalar = original_Framerate.TurboScalar;

	EnableGameFixes		= false;

	EmuOptions.EnablePatches		= true;
	EmuOptions.GS					= default_Pcsx2Config.GS;
	//EmuOptions.GS.FrameLimitEnable	= original_GS.FrameLimitEnable;	//Frame limiter is not modified by presets

	EmuOptions.Cpu					= default_Pcsx2Config.Cpu;
	EmuOptions.Gamefixes			= default_Pcsx2Config.Gamefixes;
	EmuOptions.Speedhacks			= default_Pcsx2Config.Speedhacks;
	//EmuOptions.Speedhacks	= 0; //Turn off individual hacks to make it visually clear they're not used.
	//EmuOptions.Speedhacks.vuThread	= original_SpeedHacks.vuThread;
	EnableSpeedHacks = true;

	// Actual application of current preset over the base settings which all presets use (mostly pcsx2's default values).

	bool isRateSet = false, isSkipSet = false, isMTVUSet = ignoreMTVU ? true : false; // used to prevent application of specific lower preset values on fallthrough.
	switch (n) // Settings will waterfall down to the Safe preset, then stop. So, Balanced and higher will inherit any settings through Safe.
	{
		case 5: // Mostly Harmful
			isRateSet ? 0 : (isRateSet = true, EmuOptions.Speedhacks.EECycleRate = 1); // +1 EE cyclerate
			isSkipSet ? 0 : (isSkipSet = true, EmuOptions.Speedhacks.EECycleSkip = 1); // +1 EE cycle skip
            // Fall through

		case 4: // Very Aggressive
			isRateSet ? 0 : (isRateSet = true, EmuOptions.Speedhacks.EECycleRate = -2); // -2 EE cyclerate
            // Fall through

		case 3: // Aggressive
			isRateSet ? 0 : (isRateSet = true, EmuOptions.Speedhacks.EECycleRate = -1); // -1 EE cyclerate
            // Fall throughaaad
			isMTVUSet ? 0 : (isMTVUSet = true, EmuOptions.Speedhacks.vuThread = true); // Enable MTVU
            // Fall through

		case 1: // Safe (Default)
			EmuOptions.Speedhacks.IntcStat = true;
			EmuOptions.Speedhacks.WaitLoop = true;
			EmuOptions.Speedhacks.vuFlagHack = true;

			// If waterfalling from > Safe, break to avoid MTVU disable.
			if (n > 1) break;
            // Fall through

		case 0: // Safest
			isMTVUSet ? 0 : (isMTVUSet = true, EmuOptions.Speedhacks.vuThread = false); // Disable MTVU
			break;

		default:
			Console.WriteLn("Developer Warning: Preset #%d is not implemented. (--> Using application default).", n);
	}


	EnablePresets=true;
	PresetIndex=n;

	return true;
}

void RelocateLogfile()
{

    if (!folderUtils.DoesExist(g_Conf->Folders.Logs))
    {
	    if (!folderUtils.CreateFolder(g_Conf->Folders.Logs))
	    {
		    return;
		}
    }
	std::string newLogName = ( g_Conf->Folders.Logs + "emuLog.txt");

	if( (emuLog != nullptr) && (emuLogName != newLogName) )
	{
		wxString logName(newLogName);

		Console.WriteLn( L"\nRelocating Logfile...\n\tFrom: %s\n\tTo  : %s\n", WX_STR(emuLogName), WX_STR(logName) );
		wxGetApp().DisableDiskLogging();

		fclose( emuLog );
		emuLog = nullptr;
	}

	if( emuLog == nullptr )
	{
		emuLogName = newLogName;
		emuLog = wxFopen( emuLogName, "wb" );
	}

	wxGetApp().EnableAllLogging();
}

// Parameters:
//   overwrite - this option forces the current settings to overwrite any existing settings
//      that might be saved to the configured ini/settings folder.
//
// Notes:
//   The overwrite option applies to PCSX2 options only.  Plugin option behavior will depend
//   on the plugins.
//
void AppConfig_OnChangedSettingsFolder( bool overwrite )
{
	if (!folderUtils.DoesExist(PathDefs::GetDocuments().string()))
	{
		if (!folderUtils.CreateFolder(PathDefs::GetDocuments()))
		{
			return;
		}
	}
	if (!folderUtils.DoesExist(GetSettingsFolder()))
	{
		if(folderUtils.CreateFolder(GetSettingsFolder()))
		{
			return;
		}
	}
	std::string uiFileName = GetUiSettingsFilename();
	if( overwrite )
	{
		if( folderUtils.DoesExist ( uiFileName ) && !fs::remove( uiFileName ) )
			throw Exception::AccessDenied(uiFileName)
				.SetBothMsgs(pxL("Failed to overwrite existing settings file; permission was denied."));

		std::string vmFileName = GetVmSettingsFilename();

		if( folderUtils.DoesExist( vmFileName ) && !fs::remove( vmFileName ) )
			throw Exception::AccessDenied(vmFileName)
				.SetBothMsgs(pxL("Failed to overwrite existing settings file; permission was denied."));
	}

	// Bind into wxConfigBase to allow wx to use our config internally, and delete whatever
	// comes out (cleans up prev config, if one).
	//delete wxConfigBase::Set( OpenFileConfig( (std::string)jsonFilename ) );
	//GetAppConfig()->SetRecordDefaults(true);

	if( !overwrite )
		AppLoadSettings();

	AppApplySettings();
	AppSaveSettings();//Make sure both Yaml files are created if needed.
}
// --------------------------------------------------------------------------------------
//  pxDudConfig
// --------------------------------------------------------------------------------------
// Used to handle config actions prior to the creation of the ini file (for example, the
// first time wizard).  Attempts to save ini settings are simply ignored through this
// class, which allows us to give the user a way to set everything up in the wizard, apply
// settings as usual, and only *save* something once the whole wizard is complete.
//
class pxDudConfig : public wxConfigBase
{
protected:
	wxString	m_empty;

public:
	virtual ~pxDudConfig() = default;

	virtual void SetPath(const wxString& ) {}
	virtual const wxString& GetPath() const { return m_empty; }

	virtual bool GetFirstGroup(wxString& , long& ) const { return false; }
	virtual bool GetNextGroup (wxString& , long& ) const { return false; }
	virtual bool GetFirstEntry(wxString& , long& ) const { return false; }
	virtual bool GetNextEntry (wxString& , long& ) const { return false; }
	virtual size_t GetNumberOfEntries(bool ) const  { return 0; }
	virtual size_t GetNumberOfGroups(bool ) const  { return 0; }

	virtual bool HasGroup(const wxString& ) const { return false; }
	virtual bool HasEntry(const wxString& ) const { return false; }

	virtual bool Flush(bool ) { return false; }

	virtual bool RenameEntry(const wxString&, const wxString& ) { return false; }

	virtual bool RenameGroup(const wxString&, const wxString& ) { return false; }

	virtual bool DeleteEntry(const wxString&, bool bDeleteGroupIfEmpty = true) { return false; }
	virtual bool DeleteGroup(const wxString& ) { return false; }
	virtual bool DeleteAll() { return false; }

protected:
	virtual bool DoReadString(const wxString& , wxString *) const  { return false; }
	virtual bool DoReadLong(const wxString& , long *) const  { return false; }

	virtual bool DoWriteString(const wxString& , const wxString& )  { return false; }
	virtual bool DoWriteLong(const wxString& , long )  { return false; }

#if wxUSE_BASE64
	virtual bool DoReadBinary(const wxString& key, wxMemoryBuffer* buf) const { return false; }
	virtual bool DoWriteBinary(const wxString& key, const wxMemoryBuffer& buf) { return false; }
#endif
};

static pxDudConfig _dud_config;

static void LoadUiSettings(wxConfigBase* conf)
{
	//ConLog_LoadSaveSettings(conf);
	//SysTraceLog_LoadSaveSettings();

	g_Conf->Load();

	g_Conf = std::make_unique<GuiConfig>();
	//cfg["GlobalConfig"] = g_Conf->Save();

	if( !folderUtils.DoesExist( g_Conf->CurrentIso ) )
	{
		g_Conf->CurrentIso.clear();
	}

#if defined(_WIN32)
	if( !folderUtils.DoesExist(g_Conf->Folders.RunDisc) )
	{
		//g_Conf->Folders.RunDisc.Clear();
	}
#else
	if (!folderUtils.DoesExist(g_Conf->Folders.RunDisc))
	{
		g_Conf->Folders.RunDisc.clear();
	}
#endif
	//sApp.DispatchUiSettingsEvent( loader );
}

void AppLoadSettings()
{
	if( wxGetApp().Rpc_TryInvoke(AppLoadSettings) ) return;
}

static void SaveUiSettings()
{
	if( !folderUtils.DoesExist( g_Conf->CurrentIso ) )
	{
		g_Conf->CurrentIso.clear();
	}

#if defined(_WIN32)
	if (!folderUtils.DoesExist(g_Conf->Folders.RunDisc))
	{
		g_Conf->Folders.RunDisc.clear();
	}
#else
	if (!folderUtils.DoesExist(g_Conf->Folders.RunDisc))
	{
		g_Conf->Folders.RunDisc.clear();
	}
#endif

	sApp.GetRecentIsoManager().Add( g_Conf->CurrentIso );

	g_Conf->Save();

	//std::string toSave = saver.dump();

	//sApp.DispatchUiSettingsEvent( saver );
	//fileUtils.Save(GetUiSettingsFilename(), toSave);
}

static void SaveRegSettings()
{
	bool conf_install;

	if (InstallationMode == InstallMode_Portable) return;

	// sApp. macro cannot be use because you need the return value of OpenInstallSettingsFile method
	//if( Pcsx2App* __app_ = (Pcsx2App*)wxApp::GetInstance() ) conf_install = std::unique_ptr<nlohmann::json>((*__app_).OpenInstallSettingsFile());
	//conf_install->SetRecordDefaults(false);

	//App_SaveInstallSettings( conf_install.get() );
}

void AppSaveSettings()
{
	// If multiple SaveSettings messages are requested, we want to ignore most of them.
	// Saving settings once when the GUI is idle should be fine. :)

	static std::atomic<bool> isPosted(false);

	if( !wxThread::IsMain() )
	{
		if( !isPosted.exchange(true) )
			wxGetApp().PostIdleMethod( AppSaveSettings );

		return;
	}

	Console.WriteLn("Saving config files...");

	SaveUiSettings();
	SaveRegSettings(); // save register because of PluginsFolder change

	isPosted = false;
}


// Returns the current application configuration file.  This is preferred over using
// wxConfigBase::GetAppConfig(), since it defaults to *not* creating a config file
// automatically (which is typically highly undesired behavior in our system)
wxConfigBase* GetAppConfig()
{
	return wxConfigBase::Get( false );
}


GuiConfig::GuiConfig()
	: MainGuiPosition( wxDefaultPosition )
	, SysSettingsTabName( "Cpu" )
	, McdSettingsTabName( "none" )
	, ComponentsTabName( "Plugins" )
	, AppSettingsTabName( "none" )
	, GameDatabaseTabName( "none" )
{
    LanguageId			= wxLANGUAGE_DEFAULT;
    LanguageCode		= "default";
		RecentIsoCount		= 20;
	Listbook_ImageSize	= 32;
	Toolbar_ImageSize	= 24;
	Toolbar_ShowLabels	= true;

	#ifdef __WXMSW__
	McdCompressNTFS		= true;
	#endif
	EnableSpeedHacks	= true;
	EnableGameFixes		= false;
	EnableFastBoot		= true;

	EnablePresets		= true;
	PresetIndex			= 1;

	CdvdSource			= CDVD_SourceType::Iso;

	// To be moved to FileMemoryCard pluign (someday)
	for( uint slot=0; slot<8; ++slot )
	{
		Mcd[slot].Enabled	= !FileMcd_IsMultitapSlot(slot);	// enables main 2 slots
		Mcd[slot].Filename	= FileMcd_GetDefaultName( slot );

		// Folder memory card is autodetected later.
		Mcd[slot].Type = MemoryCardType::MemoryCard_File;
	}

	GzipIsoIndexTemplate = "$(f).pindex.tmp";

}

void GuiConfig::Init()
{

	conf = new wxFileConfig();
	
	fs::path programFullPath = wxStandardPaths::Get().GetExecutablePath().ToStdString();
	std::string programDir(Path::Combine(programFullPath.parent_path(), "settings/PCSX2_ui.ini"));	
    conf->SetPath(programDir);
	
	isInit = true;

	Console.WriteLn(L"Path: " +  conf->GetPath());
}

void GuiConfig::Load()
{	
	if (!isInit)
	{
		Init();
	}

	console.Load(conf);	
	gsWindow.Load(conf);
	Filenames.Load(conf);
}


void GuiConfig::Save()
{

	if (!isInit)
	{
		Init();
	}

	console.Save(conf);
	gsWindow.Save(conf);
	Filenames.Save(conf);
    conf->Write("MainGuiPositionX", MainGuiPosition.x);
    conf->Write("MainGuiPositionY", MainGuiPosition.y);
    conf->Write("SysSettingsTabName", SysSettingsTabName);
	conf->Write("McdSettingsTabName", McdSettingsTabName);
	conf->Write("ComponentsTabName", ComponentsTabName);
	conf->Write("AppSettingsTabName", AppSettingsTabName);
	conf->Write("GameDatabaseTabName", GameDatabaseTabName);
    conf->Write("LanguageId", (int)LanguageId);
    conf->Write("LanguageCode", LanguageCode);
	conf->Flush();
}

GuiConfig::~GuiConfig()
{
   
}
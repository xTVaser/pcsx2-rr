#pragma once
#include "AppForwardDefs.h"
#include "CDVD/CDVDaccess.h"
#include "Utilities/wxGuiTools.h"
#include "Utilities/PathUtils.h"
#include "DebugTools/Debug.h"
#include "MemoryCardFile.h"
#include "Plugins.h"
#include "PathDefs.h"
#include "config/EmulatorConfig.h"

#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/fileconf.h>
#include <iostream>
#include <iomanip>
#include <memory>
#include <fstream>
#include <memory>


enum AspectRatioType
{
	AspectRatio_Stretch,
	AspectRatio_4_3,
	AspectRatio_16_9,
	AspectRatio_MaxCount
};

enum FMVAspectRatioSwitchType
{
	FMV_AspectRatio_Switch_Off,
	FMV_AspectRatio_Switch_4_3,
	FMV_AspectRatio_Switch_16_9,
	FMV_AspectRatio_Switch_MaxCount
};

enum MemoryCardType
{
	MemoryCard_None,
	MemoryCard_File,
	MemoryCard_Folder,
	MemoryCard_MaxCount
};


enum DocsModeType
{
	// uses /home/user or /cwd for the program data.  This is the default mode and is the most
	// friendly to modern computing security requirements; as it isolates all file modification
	// to a zone of the hard drive that has granted write permissions to the user.
	DocsFolder_User,

	// uses a custom location for program data. Typically the custom folder is either the
	// absolute or relative location of the program -- absolute is preferred because it is
	// considered more secure by MSW standards, due to DLL search rules.
	//
	// To enable PCSX2's "portable" mode, use this setting and specify "." for the custom
	// documents folder.
	DocsFolder_Custom,
};

namespace PathDefs
{
	// complete pathnames are returned by these functions.
	// These are used for initial default values when first configuring PCSX2, or when the
	// user checks "Use Default paths" option provided on most path selectors.  These are not
	// used otherwise, in favor of the user-configurable specifications in the ini files.

	extern fs::path GetUserLocalDataDir();
	extern fs::path GetProgramDataDir();
	extern fs::path GetDocuments();
	extern fs::path GetDocuments( DocsModeType mode );
}

extern FolderUtils      folderUtils;

extern DocsModeType		DocsFolderMode;				//
extern bool				UseDefaultSettingsFolder;	// when TRUE, pcsx2 derives the settings folder from the DocsFolderMode
extern bool				UseDefaultPluginsFolder;

extern std::string		CustomDocumentsFolder;		// allows the specification of a custom home folder for PCSX2 documents files.
extern std::string		SettingsFolder;				// dictates where the settings folder comes from, *if* UseDefaultSettingsFolder is FALSE.

extern std::string		InstallFolder;
extern std::string		PluginsFolder;

extern fs::path  GetSettingsFolder();
extern fs::path  GetVmSettingsFilename();
extern fs::path  GetUiSettingsFilename();
extern fs::path  GetUiKeysFilename();

extern fs::path GetLogFolder();
extern fs::path GetCheatsFolder();
extern fs::path GetCheatsWsFolder();

enum InstallationModeType
{
	// Use the user defined folder selections.  These can be anywhere on a user's hard drive,
	// though by default the binaries (plugins) are located in Install_Dir (registered
	// by the installer), and the user files (screenshots, inis) are in the user's documents
	// folder.  All folders are changable within the GUI.
	InstallMode_Registered,

	// In this mode, both Install_Dir and UserDocuments folders default the directory containing
	// PCSX2.exe, or the current working directory (if the PCSX2 directory could not be determined).
	// Folders cannot be changed from within the gui, however the fixed defaults can be manually
	// specified in the portable.ini by power users/devs.
	//
	// This mode is typically enabled by the presence of a 'portable.ini' in the folder.
	InstallMode_Portable,
};
bool IsPortable();

extern InstallationModeType	InstallationMode;

// ------------------------------------------------------------------------
struct ConsoleLogOptions
{
	bool Visible;
	// if true, DisplayPos is ignored and the console is automatically docked to the main window.
	bool AutoDock;
	// Display position used if AutoDock is false (ignored otherwise)
	wxPoint DisplayPosition;
	wxSize DisplaySize;

	// Size of the font in points.
	int FontSize;

	// Color theme by name!
	wxString Theme;

	ConsoleLogOptions();
	bool Save(wxConfigBase* conf);
	void Load(wxConfigBase* conf);
};

	// ------------------------------------------------------------------------
	struct FolderOptions
	{
		bool
			UseDefaultBios,
			UseDefaultSnapshots,
			UseDefaultSavestates,
			UseDefaultMemoryCards,
			UseDefaultLogs,
			UseDefaultLangs,
			UseDefaultCheats,
			UseDefaultCheatsWS;

	std::string
		Bios,
		Snapshots,
		Savestates,
		MemoryCards,
		Langs,
		Logs,
		Cheats,
		CheatsWS;

	fs::path RunIso;		// last used location for Iso loading.
	fs::path RunELF;		// last used location for ELF loading.
	fs::path RunDisc;		// last used location for Disc loading.

	FolderOptions();
	bool Save(wxConfigBase* conf);
	void Load(wxConfigBase* conf);
	void ApplyDefaults();

	void Set( FoldersEnum_t folderidx, const std::string& src, bool useDefault );

	const std::string& operator[]( FoldersEnum_t folderidx ) const;
	std::string& operator[]( FoldersEnum_t folderidx );
	bool IsDefault( FoldersEnum_t folderidx ) const;
};


#ifndef DISABLE_RECORDING
	struct InputRecordingOptions
	{
		wxPoint		VirtualPadPosition;

		InputRecordingOptions();
		bool Save( wxConfigBase* conf);
	};
#endif
// ------------------------------------------------------------------------
// The GS window receives much love from the land of Options and Settings.
//
struct GSWindowOptions
{
	// Closes the GS/Video port on escape (good for fullscreen activity)
	bool CloseOnEsc;

	bool DefaultToFullscreen;
	bool AlwaysHideMouse;
	bool DisableResizeBorders;
	bool DisableScreenSaver;

	AspectRatioType AspectRatio;
	FMVAspectRatioSwitchType FMVAspectRatioSwitch;
	float Zoom;
	float StretchY;
	float OffsetX;
	float OffsetY;

	wxSize WindowSize;
	wxPoint WindowPos;
	bool IsMaximized;
	bool IsFullscreen;
	bool EnableVsyncWindowFlag;

	bool IsToggleFullscreenOnDoubleClick;

	GSWindowOptions();

	bool Save(wxConfigBase* conf);
	void Load(wxConfigBase* conf);
	void SanityCheck();
};

	// ------------------------------------------------------------------------
struct FilenameOptions
{
	wxString Bios;
	wxString Plugins[PluginId_Count];

	bool Save(wxConfigBase* conf);
	void Load(wxConfigBase* conf);

	const std::string& operator[](PluginsEnum_t pluginidx) const;
};

// ------------------------------------------------------------------------
// Options struct for each memory card.
//
struct McdOptions
{
	wxFileName	Filename;	// user-configured location of this memory card
	bool		Enabled;	// memory card enabled (if false, memcard will not show up in-game)
	MemoryCardType Type;	// the memory card implementation that should be used
};

struct FramerateOptions
{
	bool		SkipOnLimit;
	bool		SkipOnTurbo;

	float	NominalScalar;
	float	TurboScalar;
	float	SlomoScalar;

	FramerateOptions();

	bool Save(wxConfigBase* conf);
	void Load(wxConfigBase* conf);
	void SanityCheck();
};

struct UiTemplateOptions 
{
	UiTemplateOptions();
			
	bool Save(wxConfigBase* conf);
	void Load(wxConfigBase* conf);

	std::string LimiterUnlimited;
	std::string LimiterTurbo;
	std::string LimiterSlowmo;
	std::string LimiterNormal;
	std::string OutputFrame;
	std::string OutputField;
	std::string OutputProgressive;
	std::string OutputInterlaced;
	std::string Paused;
	std::string TitleTemplate;
#ifndef DISABLE_RECORDING
	std::string RecordingTemplate;
#endif
};

class GuiConfig
{	

private:
	// The Configurator
    wxConfigBase* conf;
	bool isInit = false;

public:	
	ConsoleLogOptions console;
	InputRecordingOptions Input;	
	GSWindowOptions gsWindow;
	wxPoint	MainGuiPosition;

	// Because remembering the last used tab on the settings panel is cool (tab is remembered
	// by it's UTF/ASCII name).
	wxString	SysSettingsTabName;
	wxString	McdSettingsTabName;
	wxString	ComponentsTabName;
	wxString	AppSettingsTabName;
	wxString	GameDatabaseTabName;

	fs::path				CurrentIso;
    std::string				CurrentBlockdump;
	std::string				CurrentELF;
	std::string				CurrentIRX;
	CDVD_SourceType			CdvdSource;
	std::string				CurrentGameArgs;

	std::string FullpathToBios() const;
	std::string FullpathToMcd( uint slot ) const;
	std::string FullpathTo( PluginsEnum_t pluginId ) const;

	bool FullpathMatchTest( PluginsEnum_t pluginId, const wxString& cmpto );

	bool Save(wxConfigBase* conf);
	void Load(wxConfigBase* conf);
	bool SaveRootItems(wxConfigBase* conf);
	void LoadRootItems(wxConfigBase* conf);
	bool SaveMemcards(wxConfigBase* conf);
	void LoadMemcards(wxConfigBase* conf);

	static int  GetMaxPresetIndex();
    static bool isOkGetPresetTextAndColor(int n, std::string& label, wxColor& c);

	bool        IsOkApplyPreset(int n, bool ignoreMTVU);


	//The next 2 flags are used with ApplyConfigToGui which the presets system use:

	//Indicates that the scope is only for preset-related items.
	static const int APPLY_FLAG_FROM_PRESET			= 0x01;

	//Indicates that the change should manually propagate to sub items because it's called directly and not as an event.
	//Currently used by some panels which contain sub-panels which are affected by presets.
	static const int APPLY_FLAG_MANUALLY_PROPAGATE	= 0x02;

	int			RecentIsoCount;		// number of files displayed in the Recent Isos list.

	// Specifies the size of icons used in Listbooks; specifically the PCSX2 Properties dialog box.
	// Realistic values range from 96x96 to 24x24.
	int			Listbook_ImageSize;

	// Specifies the size of each toolbar icon, in pixels (any value >= 2 is valid, but realistically
	// values should be betweeUiTemplateOptionsn 64 and 16 for usability reasons)
	int			Toolbar_ImageSize;

	// Enables display of toolbar text labels.
	bool		Toolbar_ShowLabels;

	// uses automatic ntfs compression when creating new memory cards (Win32 only)
#ifdef __WXMSW__
	bool		McdCompressNTFS;
#endif

	// Master toggle for enabling or disabling all speedhacks in one fail-free swoop.
	// (the toggle is applied when a new EmuConfig is sent through AppCoreThread::ApplySettings)
	bool		EnableSpeedHacks;
	bool		EnableGameFixes;
	bool		EnableFastBoot;

	// Presets try to prevent users from overwhelming when they want to change settings (usually to make a game run faster).
	// The presets allow to modify the balance between emulation accuracy and emulation speed using a pseudo-linear control.
	// It's pseudo since there's no way to arrange groups of all of pcsx2's settings such that each next group makes it slighty faster and slightly less compatiible for all games.
	//However, By carefully selecting these preset config groups, it's hopefully possible to achieve this goal for a reasonable percentage (hopefully above 50%) of the games.
	//when presets are enabled, the user has practically no control over the emulation settings, and can only choose the preset to use.

	// The next 2 vars enable/disable presets alltogether, and select/reflect current preset, respectively.
	bool		EnablePresets;
	int			PresetIndex;

	bool		AskOnBoot;

	// Memorycard options - first 2 are default slots, last 6 are multitap 1 and 2
	// slots (3 each)
	McdOptions				Mcd[8];
	std::string				GzipIsoIndexTemplate; // for quick-access index with gzipped ISO

	FolderOptions			Folders;
	FilenameOptions			BaseFilenames;
	FramerateOptions		Framerate;
#ifndef DISABLE_RECORDING
	InputRecordingOptions   inputRecording;
#endif
	UiTemplateOptions		Templates;

	// Currently selected language ID -- wxWidgets version-specific identifier.  This is one side of
	// a two-part configuration that also includes LanguageCode.
	wxLanguage	LanguageId;

	// Current language in use (correlates to the universal language codes, such as "en_US", "de_DE", etc).
	// This code is not always unique, which is why we use the language ID also.
	wxString	LanguageCode;

	GuiConfig();
	void Init();
	void Load();
	void Save();
	~GuiConfig();
};

extern void AppLoadSettings();
extern void AppSaveSettings();
extern void AppApplySettings( const GuiConfig* oldconf=NULL );

extern wxFileConfig App_LoadSaveInstallSettings( );

// TODO
//extern void App_SaveInstallSettings( YAML::Node yaml );
//extern void App_LoadInstallSettings( YAML::Node yaml );

extern wxFileConfig ConLog_LoadSaveSettings(wxConfigBase* conf);
extern wxFileConfig SysTraceLog_LoadSaveSettings();


// TODO - 
// extern bool OpenFileConfig( std::string filename );
extern void RelocateLogfile();
extern void AppConfig_OnChangedSettingsFolder( bool overwrite =  false );
extern wxConfigBase* GetAppConfig();

extern std::unique_ptr<GuiConfig> g_Conf;

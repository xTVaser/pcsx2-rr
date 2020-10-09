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
#include "MainFrame.h"
#include "Utilities/json.hpp"
#include "Utilities/PathUtils.h"
#include "Dialogs/ModalPopups.h"
#include "AppConfig.h"
#include <wx/stdpaths.h>


#ifdef __WXMSW__
#include "wx/msw/regconf.h"
#endif

DocsModeType			DocsFolderMode = DocsFolder_User;
bool					UseDefaultSettingsFolder = true;
bool					UseDefaultPluginsFolder = true;

std::vector<std::string> ErrorFolders;


std::string				CustomDocumentsFolder;
std::string				SettingsFolder;

std::string				InstallFolder;
std::string				PluginsFolder;
	
nlohmann::json stream;
	
const std::string PermissionFolders[] =
{
	"json",
	"memcards",
	"sstates",
	"snapshots",
	"logs",
	"cheats_ws",
	#ifdef PCSX2_DEVBUILD
	"dumps",
	#endif
};

// The UserLocalData folder can be redefined depending on whether or not PCSX2 is in
// "portable install" mode or not.  when PCSX2 has been configured for portable install, the
// UserLocalData folder is the current working directory.
InstallationModeType		InstallationMode;

static fs::path GetPortableJsonPath()
{
	fs::path programDir = Path::GetExecutablePath().parent_path();
	return Path::Combine(programDir, "portable.json");
}

static wxString GetMsg_PortableModeRights()
{
	return pxE( L"Please ensure that these folders are created and that your user account is granted write permissions to them -- or re-run PCSX2 with elevated (administrator) rights, which should grant PCSX2 the ability to create the necessary folders itself.  If you do not have elevated rights on this computer, then you will need to switch to User Documents mode (click button below)."
	);
};

bool Pcsx2App::TestUserPermissionsRights( const std::string& testFolder)
{
 
	std::string createme, accessme;


	for (int i = 0; i < 5; ++i)
	{
		fs::path folder = Path::Combine(testFolder, PermissionFolders[i]);
	
		if (!folderUtils.DoesExist(folder))
        {
		    if (!folderUtils.CreateFolder(folder))
		    {
			    ErrorFolders.push_back(folder); 
			    //wxFileConfig* OpenFileConfig( const std::string& filename );
		    }
        }
	}

	for (int i = 0; i < ErrorFolders.size(); i++)
	{
		Console.WriteLn((wxString)_("The following folders are missing and cannot be created:") + L"\n" + ErrorFolders[i]);
	}

	if (ErrorFolders.size() > 0)
	{
		return false;
	}

	else
	{
		return true;
	}
	

}

// Portable installations are assumed to be run in either administrator rights mode, or run
// from "insecure media" such as a removable flash drive.  In these cases, the default path for
// PCSX2 user documents becomes ".", which is the current working directory.
//
// Portable installation mode is typically enabled via the presence of an json file in the
// same directory that PCSX2 is installed to.
//
bool Pcsx2App::TestForPortableInstall()
{
	InstallationMode = 	InstallMode_Portable;

	fs::path portableJsonFile = GetPortableJsonPath();
	std::string portableDocsFolder = portableJsonFile.parent_path();

	std::cout << "PATH: " << portableJsonFile << std::endl;
	bool isPortable = OpenFileConfig( portableJsonFile);

	if (Startup.PortableMode || isPortable)
	{
		if (Startup.PortableMode)
			Console.WriteLn( L"(UserMode) Portable mode requested via commandline switch!" );
		else
		{
			wxString temp = portableJsonFile.string();
            Console.WriteLn( L"(UserMode) Found portable install json @ %s", WX_STR(temp));
		}
		// Just because the portable json file exists doesn't mean we can actually run in portable
		// mode.  In order to determine our read/write permissions to the PCSX2, we must try to
		// modify the configured documents folder, and catch any ensuing error.
		
		if (!TestUserPermissionsRights(portableDocsFolder))
		{
			Console.WriteLn(L"Install Mode Activated");
			return false;
		}


		// Success -- all user-based folders have write access.  PCSX2 should be able to run error-free!
		// Force-set the custom documents mode, and set the

		InstallationMode = InstallMode_Portable;
		DocsFolderMode = DocsFolder_Custom;
		CustomDocumentsFolder = portableDocsFolder;
		return isPortable;
	}

}
// Reset RunWizard so the FTWizard is run again on next PCSX2 start.
void Pcsx2App::WipeUserModeSettings()
{
	if (InstallationMode == InstallMode_Portable)
	{
		// Remove the portable.json entry "RunWizard" conforming to this instance of PCSX2.
		std::string portableJsonFile( GetPortableJsonPath() );
		bool test = OpenFileConfig( portableJsonFile );
		json = fileUtils.GetStream();
		json["RunWizard"] = 0;
	}
	else
	{
		// Remove the registry entry "RunWizard" conforming to this instance of PCSX2.
		bool conf_install = OpenInstallSettingsFile();		
		json = fileUtils.GetStream();
		json["RunWizard"] = 0;
	}
}

static void DoFirstTimeWizard()
{
	// first time startup, so give the user the choice of user mode:
	while(true)
	{
		// PCSX2's FTWizard allows improptu restarting of the wizard without cancellation.
		// This is typically used to change the user's language selection.

		FirstTimeWizard wiz( NULL );
		if( wiz.RunWizard( wiz.GetFirstPage() ) ) break;
		if (wiz.GetReturnCode() != pxID_RestartWizard)
			throw Exception::StartupAborted( "User canceled FirstTime Wizard." );

		Console.WriteLn( Color_StrongBlack, "Restarting First Time Wizard!" );
	}
}

bool Pcsx2App::OpenInstallSettingsFile()
{
	// Implementation Notes:
	//
	// As of 0.9.8 and beyond, PCSX2's versioning should be strong enough to base json and
	// plugin compatibility on version information alone.  This in turn allows us to ditch
	// the old system (CWD-based json file mess) in favor of a system that simply stores
	// most core application-level settings in the registry.

	//nlohmann::json conf_install;

	InstallationMode = InstallationModeType::InstallMode_Registered;


#ifdef __WXMSW__
	//conf_install = std::unique_ptr<nlohmann::json>(new wxRegConfig());
#else
	// FIXME!!  Linux / Mac
	// Where the heck should this information be stored?

	fs::path usrlocaldir = PathDefs::GetUserLocalDataDir();
	//fs::path usrlocaldir( wxStandardPaths::Get().GetDataDir() );
	if( !fs::exists(usrlocaldir))
	{
		Console.WriteLn( "Creating UserLocalData folder: " + (std::string)usrlocaldir );
		if (!folderUtils.CreateFolder(usrlocaldir))
		{
			return false;
		}
	}

	std::string usermodefile = (GetAppName().ToStdString() + "-reg.json" );

	std::cout << "USERMODE: " << usermodefile << std::endl;

	if(!fileUtils.Load(usermodefile))
	{
		return false;
	}
#endif

	return true;
}


void Pcsx2App::ForceFirstTimeWizardOnNextRun()
{
	bool conf_install;
	conf_install = TestForPortableInstall();
	
	if (!conf_install)
		conf_install = OpenInstallSettingsFile();

	json["RunWizard"] = true;
}

void Pcsx2App::EstablishAppUserMode()
{
	// TODO - stop mutating the json directly, serialize and deserialize!

	bool conf_install = TestForPortableInstall();

	if (!conf_install)
		conf_install = OpenInstallSettingsFile();

	nlohmann::json newJson = fileUtils.GetStream();

	//  Run the First Time Wizard!
	// ----------------------------
	// Wizard is only run once.  The status of the wizard having been run is stored in
	// the installation json file, which can be either the portable install (useful for admins)
	// or the registry/user local documents position.

	bool runWizard = false;
	if (newJson["RunWizard"])
		runWizard = true;

	App_LoadInstallSettings( newJson);

	if( !Startup.ForceWizard)
	{
		AppConfig_OnChangedSettingsFolder( false );
		return;
	}

	DoFirstTimeWizard();

	// Save user's new settings
	App_SaveInstallSettings( newJson);
	AppConfig_OnChangedSettingsFolder( true );
	AppSaveSettings();

	json = newJson;

	// Wizard completed successfully, so let's not torture the user with this crap again!
	
	// TODO - stawp

    if (InstallationMode == InstallationModeType::InstallMode_Portable)
    {
		newJson["RunWizard"] = false;

        fileUtils.Save(GetPortableJsonPath(), newJson.dump());
    }

}


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
	
PathUtils path;

//nlohmann::json json;
	
std::ifstream in;

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
	fs::path programFullPath = wxStandardPaths::Get().GetExecutablePath().ToStdString();
	fs::path programDir( programFullPath.parent_path());
	return Path::Combine(programDir, "portable.json");
}

static wxString GetMsg_PortableModeRights()
{
	return pxE( L"Please ensure that these folders are created and that your user account is granted write permissions to them -- or re-run PCSX2 with elevated (administrator) rights, which should grant PCSX2 the ability to create the necessary folders itself.  If you do not have elevated rights on this computer, then you will need to switch to User Documents mode (click button below)."
	);
};

bool Pcsx2App::TestUserPermissionsRights( const std::string& testFolder, std::string& createFailedStr, std::string& accessFailedStr )
{

	std::string createme, accessme;


	for (int i = 0; i < 5; ++i)
	{
		fs::path folder = Path::Combine(testFolder, PermissionFolders[i]);
	
		if (!path.DoesExist(folder))
        {
		    if (!path.CreateFolder(folder))
		    {
			    ErrorFolders.push_back(folder); 
			    wxFileConfig* OpenFileConfig( const std::string& filename );
		    }
        }
	}

	for (int i = 0; i < ErrorFolders.size(); i++)
	{
		createFailedStr = (wxString)_("The following folders are missing and cannot be created:") + L"\n" + ErrorFolders[i];
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
nlohmann::json* Pcsx2App::TestForPortableInstall()
{
	InstallationMode = 	InstallMode_Portable;

	fs::path portableJsonFile = GetPortableJsonPath();
	fs::path portableDocsFolder = portableJsonFile.parent_path();
	std::string FilenameStr = portableJsonFile.string();

	std::cout << "PATH: " << FilenameStr << std::endl;

	if (Startup.PortableMode || !portableJsonFile.empty())
	{
		if (Startup.PortableMode)
			Console.WriteLn( L"(UserMode) Portable mode requested via commandline switch!" );
		else
			Console.WriteLn( L"(UserMode) Found portable install json @ %s", (wxString)FilenameStr);

		// Just because the portable json file exists doesn't mean we can actually run in portable
		// mode.  In order to determine our read/write permissions to the PCSX2, we must try to
		// modify the configured documents folder, and catch any ensuing error.
		std::unique_ptr<nlohmann::json> conf_portable( OpenFileConfig( FilenameStr ) );
		
		while( true ) // ?? why a whole loop here
		{
			std::string accessFailedStr, createFailedStr;
			if (TestUserPermissionsRights( portableDocsFolder, createFailedStr, accessFailedStr )) break;

			wxDialogWithHelpers dialog( NULL, AddAppName(_("Portable mode error - %s")) );

			wxTextCtrl* scrollText = new wxTextCtrl(
				&dialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
				wxTE_READONLY | wxTE_MULTILINE | wxTE_WORDWRAP
			);

			if (!path.Empty(createFailedStr))
				scrollText->AppendText( createFailedStr + L"\n" );

			dialog += dialog.Heading( _("PCSX2 has been installed as a portable application but cannot run due to the following errors:" ) );
			dialog += scrollText | pxExpand.Border(wxALL, 16);
			dialog += 6;
			dialog += dialog.Text( GetMsg_PortableModeRights() );

			// [TODO] : Add url for platform-relevant user permissions tutorials?  (low priority)

			wxWindowID result = pxIssueConfirmation( dialog,
				MsgButtons().Retry().Cancel().Custom(_("Switch to User Documents Mode"), "switchmode")
			);

			switch (result)
			{
				case wxID_CANCEL:
					throw Exception::StartupAborted( L"User canceled portable mode due to insufficient user access/permissions." );

				case wxID_RETRY:
					// do nothing (continues while loop)
				break;

				case pxID_CUSTOM:
					wxDialogWithHelpers dialog2( NULL, AddAppName(_("%s is switching to local install mode.")) );
					dialog2 += dialog2.Heading( _("Try to remove the file called \"portable.json\" from your installation directory manually." ) );
					dialog2 += 6;
					pxIssueConfirmation( dialog2, MsgButtons().OK() );

					return nullptr;
			}

		}

		// Success -- all user-based folders have write access.  PCSX2 should be able to run error-free!
		// Force-set the custom documents mode, and set the

		InstallationMode = InstallMode_Portable;
		DocsFolderMode = DocsFolder_Custom;
		CustomDocumentsFolder = portableDocsFolder;
		return conf_portable.release();
	}

}
// Reset RunWizard so the FTWizard is run again on next PCSX2 start.
void Pcsx2App::WipeUserModeSettings()
{
	if (InstallationMode == InstallMode_Portable)
	{
		// Remove the portable.json entry "RunWizard" conforming to this instance of PCSX2.
		std::string portableJsonFile( GetPortableJsonPath() );
		std::unique_ptr<nlohmann::json> conf_portable( OpenFileConfig( portableJsonFile ) );
		json = *conf_portable.get();
		json["RunWizard"] = 0;
	}
	else
	{
		// Remove the registry entry "RunWizard" conforming to this instance of PCSX2.
		std::unique_ptr<nlohmann::json> conf_install( OpenInstallSettingsFile() );		
		json = *conf_install.get();
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

nlohmann::json* Pcsx2App::OpenInstallSettingsFile()
{
	// Implementation Notes:
	//
	// As of 0.9.8 and beyond, PCSX2's versioning should be strong enough to base json and
	// plugin compatibility on version information alone.  This in turn allows us to ditch
	// the old system (CWD-based json file mess) in favor of a system that simply stores
	// most core application-level settings in the registry.

	std::unique_ptr<nlohmann::json> conf_install;

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
		path.CreateFolder(usrlocaldir);
	}

	std::string usermodefile = (GetAppName().ToStdString() + "-reg.json" );

	std::cout << "USERMODE: " << usermodefile << std::endl;

	in.open(usermodefile);
	json = nlohmann::json::parse(in);

#endif

	return &json;
}


void Pcsx2App::ForceFirstTimeWizardOnNextRun()
{
	std::unique_ptr<nlohmann::json> conf_install;
	conf_install = std::unique_ptr<nlohmann::json>(TestForPortableInstall());
	
	if (!conf_install)
		conf_install = std::unique_ptr<nlohmann::json>(OpenInstallSettingsFile());

	json["RunWizard"] = true;
}

void Pcsx2App::EstablishAppUserMode()
{
	wxGetApp().GetGameDatabase();



	std::unique_ptr<nlohmann::json> conf_install;
	conf_install = std::unique_ptr<nlohmann::json>(TestForPortableInstall());

	//if (!conf_install)
		//conf_install = std::unique_ptr<nlohmann::json>(OpenInstallSettingsFile());

	//conf_install->SetRecordDefaults(false);

	//  Run the First Time Wizard!
	// ----------------------------
	// Wizard is only run once.  The status of the wizard having been run is stored in
	// the installation json file, which can be either the portable install (useful for admins)
	// or the registry/user local documents position.

	bool runWiz = true;
	//if json["RunWizard"] &runWiz, true );

	App_LoadInstallSettings( conf_install.get());

	if( !Startup.ForceWizard && !runWiz )
	{
		AppConfig_OnChangedSettingsFolder( false );
		return;
	}

	DoFirstTimeWizard();

	// Save user's new settings
	App_SaveInstallSettings( conf_install.get());
	AppConfig_OnChangedSettingsFolder( true );
	AppSaveSettings();

	// Wizard completed successfully, so let's not torture the user with this crap again!
<<<<<<< HEAD
	json["RunWizard"] = false;
=======
//	json["RunWizard"] = 0;
>>>>>>> yaml/yaml-cpp
}


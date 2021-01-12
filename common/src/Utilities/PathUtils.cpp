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
#include "Path.h"
#include "PathUtils.h"
#include <wx/file.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include "ghc/filesystem.h"
#include <wx/utils.h>

// ---------------------------------------------------------------------------------
//  wxDirName (implementations)
// ---------------------------------------------------------------------------------

wxFileName wxDirName::Combine(const wxFileName &right) const
{
    pxAssertMsg(IsDir(), L"Warning: Malformed directory name detected during wxDirName concatenation.");
    if (right.IsAbsolute())
        return right;

    // Append any directory parts from right, and then set the filename.
    // Except we can't do that because our m_members are private (argh!) and there is no API
    // for getting each component of the path.  So instead let's use Normalize:

    wxFileName result(right);
    result.Normalize(wxPATH_NORM_ENV_VARS | wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE, GetPath());
    return result;
}

wxDirName wxDirName::Combine(const wxDirName &right) const
{
    pxAssertMsg(IsDir() && right.IsDir(), L"Warning: Malformed directory name detected during wDirName concatenation.");

    wxDirName result(right);
    result.Normalize(wxPATH_NORM_ENV_VARS | wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE, GetPath());
    return result;
}

wxDirName &wxDirName::Normalize(int flags, const wxString &cwd)
{
    pxAssertMsg(IsDir(), L"Warning: Malformed directory name detected during wDirName normalization.");
    if (!wxFileName::Normalize(flags, cwd))
        throw Exception::ParseError().SetDiagMsg(L"wxDirName::Normalize operation failed.");
    return *this;
}

void wxDirName::Rmdir()
{
    if (!Exists())
        return;
    wxFileName::Rmdir();
    // TODO : Throw exception if operation failed?  Do we care?
}

bool wxDirName::Mkdir()
{
// wxWidgets recurses directory creation for us.

// only exist in wx2.9 and above
#ifndef wxS_DIR_DEFAULT
#define wxS_DIR_DEFAULT 0777
#endif

    if (Exists())
        return true;
    return wxFileName::Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
}


// ---------------------------------------------------------------------------------
//  Path namespace (wxFileName helpers)
// ---------------------------------------------------------------------------------


bool Path::IsRelative(const std::string &path)
{
	return fs::path(path).is_relative();
}

// Returns -1 if the file does not exist.
s64 Path::GetFileSize(const std::string &path)
{
    if (!fs::exists(path.c_str()))
        return -1;
    return (s64)fs::file_size(path);
}


wxString Path::Normalize(const wxString &src)
{
    wxFileName normalize(src);
    normalize.Normalize();
    return normalize.GetFullPath();
}

wxString Path::Normalize(const wxDirName &src)
{
    return wxDirName(src).Normalize().ToString();
}

std::string Path::MakeAbsolute(const std::string &src)
{
    return ghc::filesystem::absolute(src);
}

// Concatenates two pathnames together, inserting delimiters (backslash on win32)
// as needed! Assumes the 'dest' is allocated to at least g_MaxPath length.
//
fs::path Path::Combine(fs::path &srcPath, fs::path &srcFile)
{
    return (srcPath / srcFile).make_preferred();
}

std::string Path::Combine(const std::string &srcPath, const std::string &srcFile)
{
    fs::path srcP = srcPath;
    fs::path srcF = srcFile;
    return (srcP / srcF).make_preferred();
}
// Replaces the extension of the file with the one given.
// This function works for path names as well as file names.
std::string Path::ReplaceExtension(const wxString &src, const wxString &ext)
{
    wxFileName jojo(src);
    jojo.SetExt(ext);
    return jojo.GetFullPath().ToStdString();
}

std::string Path::ReplaceFilename(const wxString &src, const wxString &newfilename)
{
    wxFileName jojo(src);
    jojo.SetFullName(newfilename);
    return jojo.GetFullPath().ToStdString();
}

std::string Path::GetFilename(const std::string &src)
{
    return ghc::filesystem::absolute(src);
}

wxString Path::GetFilenameWithoutExt(const wxString &src)
{
    return wxFileName(src).GetName();
}

std::string Path::GetDirectory(const std::string &src)
{
    return src;
}

fs::path Path::GetExecutableDirectory()
{
	fs::path exePath(wxStandardPaths::Get().GetExecutablePath().ToStdWstring());
	return exePath.parent_path();
}

fs::path Path::getPath(fs::path p, bool isPort)
{
    if (isPort)
    {
        return fs::relative(p);
    }
    else
    {
        return fs::absolute(p);
    }
}

// returns the base/root directory of the given path.
// Example /this/that/something.txt -> dest == "/"
std::string Path::GetRootDirectory(const wxString &src)
{
    size_t pos = src.find_first_of(wxFileName::GetPathSeparators());
    if (pos == 0)
        return std::string();
    else
        return fs::path(std::string(src.begin(), src.begin() + pos));
}

// ------------------------------------------------------------------------
// Launches the specified file according to its mime type
//
void pxLaunch(const wxString &filename)
{
    wxLaunchDefaultBrowser(filename);
}

void pxLaunch(const char *filename)
{
    pxLaunch(fromUTF8(filename));
}

// ------------------------------------------------------------------------
// Launches a file explorer window on the specified path.  If the given path is not
// a qualified URI (with a prefix:// ), file:// is automatically prepended.  This
// bypasses wxWidgets internal filename checking, which can end up launching things
// through browser more often than desired.
//
void pxExplore(const wxString &path)
{
    wxLaunchDefaultBrowser(!path.Contains(L"://") ? L"file://" + path : path);
}

void pxExplore(const char *path)
{
    pxExplore(fromUTF8(path));
}

bool Path::CreateFolder(const fs::path& path)
{
    return fs::create_directories(path); // An attempt to create the User mode Dir which already exists
}

bool Path::DoesExist(std::string path)
{
    return fs::exists(fs::path(path).make_preferred());
}

bool Path::DoesExist(fs::path path)
{
	try
	{
		return fs::exists(path.make_preferred());
	}
	catch (fs::filesystem_error ex)
	{
		return false;
	}
}

wxString Path::ToWxString(const fs::path& path)
{
#ifdef _WIN32
	return wxString(path.wstring());
#else
	return wxString(path.string());
#endif
}

bool Path::Empty(std::string path)
{
    return fs::is_empty(path);
}

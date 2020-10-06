#ifndef PATHUTILS_H
#define PATHUTILS_H
#include <fstream>
#include <iostream>
#include <iomanip>
#include "FixedPointTypes.inl"
#include "yaml-cpp/yaml.h"
#include "filesystem.hpp"
#include "json.hpp"
#include "Path.h"

namespace fs = ghc::filesystem;

class PathUtils
{
    private:
    std::ifstream in;
    std::ofstream out;
    public:
    bool CreateFolder(std::string path);
    bool Empty(std::string); // Is the folder empty
    bool DoesExist(std::string path); // Does the path exist
    bool DoesExist(fs::path path); // Does the path exist

    void Open(fs::path toOpen);
    void Save(fs::path toSave, nlohmann::json& stream); // TO DO use yaml
    nlohmann::json *Load(fs::path toLoad); // TO DO use yaml
};

namespace Path
{
extern bool IsRelative(const std::string &path);
extern s64 GetFileSize(const std::string &path);

extern wxString Normalize(const wxString &srcpath);
extern wxString Normalize(const wxDirName &srcpath);
extern std::string MakeAbsolute(const std::string &srcpath);

extern fs::path Combine(fs::path srcPath, fs::path srcFile);
//extern std::string Combine(const wxDirName &srcPath, const wxFileName &srcFile);
extern std::string Combine(const std::string &srcPath, const std::string &srcFile);
extern std::string ReplaceExtension(const wxString &src, const wxString &ext);
extern std::string ReplaceFilename(const wxString &src, const wxString &newfilename);
extern std::string GetFilename(const std::string &src);
extern std::string GetDirectory(const std::string &src);
extern wxString GetFilenameWithoutExt(const wxString &src);
extern std::string GetRootDirectory(const wxString &src);
}

#endif
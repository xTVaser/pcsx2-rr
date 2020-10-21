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

class FolderUtils
{
    protected:
    fs::path folder;

    public:
    bool CreateFolder(std::string path);
    bool Empty(std::string); // Is the folder empty
    bool DoesExist(std::string path); // Does the path exist
    bool DoesExist(fs::path path); // Does the path exist
};

class iFile
{

protected:
    std::ifstream in;
    std::ofstream out;
    FolderUtils folder;
    std::string data;


public:
    virtual bool Save(fs::path toSave, std::string) = 0;
    virtual bool Load(fs::path toLoad) = 0;
};

class YamlUtils : iFile
{
private:
    YAML::Node stream;
public:
    bool Save(fs::path toSave, std::string) override;
    bool Load(fs::path toLoad) override;
    YAML::Node GetStream();
};


class JsonUtils : iFile
{

private:
    nlohmann::json stream;
public:
    bool Save(fs::path toSave, std::string) override;
    bool Load(fs::path toLoad) override;
    nlohmann::json GetStream();
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
extern fs::path GetExecutableDirectory();
}

#endif
#ifndef PATHUTILS_H
#define PATHUTILS_H
#include "filesystem.hpp"
namespace fs = ghc::filesystem;
class PathUtils
{
    private:

    public:

    bool CreateFolder(std::string path);
    bool Empty(); // Is the folder empty
    bool DoesExist(std::string &path); // Does the path exist
};
#endif
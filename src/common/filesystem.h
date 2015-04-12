#ifndef COMMON_FILESYSTEM_H
#define COMMON_FILESYSTEM_H

#include <string>


namespace Airwave {


class FileSystem {
public:
	static std::string realPath(const std::string& path);
	static bool isFileExists(const std::string& path);
	static bool isDirExists(const std::string &path);
	static bool makePath(const std::string& path);
	static bool makeDir(const std::string& path);
	static std::string fullNameFromPath(const std::string& fileName);
	static std::string baseName(const std::string& path);
};


} // namespace Airwave


#endif // COMMON_FILESYSTEM_H

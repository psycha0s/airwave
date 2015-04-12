#include "filesystem.h"

#include <vector>
#include <pwd.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/stat.h>


namespace Airwave {


std::string FileSystem::realPath(const std::string& path)
{
	if(!path.empty()) {
		std::string result;

		if(path[0] == '~') {
			struct passwd* pw = getpwuid(getuid());
			result = pw->pw_dir;
			result += path.substr(1);
		}
		else {
			result = path;
		}

		char buffer[PATH_MAX];

		if(realpath(result.c_str(), buffer))
			return std::string(buffer);
	}

	return std::string();
}


bool FileSystem::isFileExists(const std::string& path)
{
	return access(path.c_str(), F_OK) != -1;
}


bool FileSystem::isDirExists(const std::string& path)
{
	struct stat info;
	return stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode);
}


bool FileSystem::makePath(const std::string& path)
{
	std::size_t begin = 0;
	std::size_t pos;
	std::string dir;

	while(begin < path.length()) {
		pos = path.find('/', begin);

		if(pos == std::string::npos) {
			dir = path;
			pos = path.length();
		}
		else {
			dir = path.substr(0, pos + 1);
		}

		if(!isDirExists(dir))
			break;

		begin = pos + 1;
	}

	while(begin < path.length()) {
		pos = path.find('/', begin);

		if(pos == std::string::npos) {
			dir = path;
			pos = path.length();
		}
		else {
			dir = path.substr(0, pos + 1);
		}

		if(!makeDir(dir))
			return false;

		begin = pos + 1;
	}

	return true;
}


bool FileSystem::makeDir(const std::string& path)
{
	mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

	if(mkdir(path.c_str(), mode) != 0)
		return false;

	return true;
}


std::string FileSystem::fullNameFromPath(const std::string& fileName)
{
	std::string path = getenv("PATH");
	size_t begin = 0;
	size_t pos = begin;

	while(pos < path.length()) {
		pos = path.find(':', begin);

		std::string fullName = path.substr(begin, pos - begin);
		fullName += '/' + fileName;
		if(isFileExists(fullName))
			return fullName;

		begin = pos + 1;
	}

	return fileName;
}


std::string FileSystem::baseName(const std::string& path)
{
	size_t pos = path.rfind('/');
	if(pos != std::string::npos)
		return path.substr(pos + 1);

	return path;
}


} // namespace Airwave

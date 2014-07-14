#include "linkmanager.h"

#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include "config.h"
#include "filesystem.h"
#include "logger.h"


namespace Airwave {


LinkManager::LinkManager() :
	isInitialized_(false)
{
	magic_ = magic_open(MAGIC_NONE);
	if(!magic_) {
		LOG("Unable to initialize libmagic.");
		return;
	}

	if(magic_load(magic_, nullptr) != 0) {
		LOG("libmagic error: %s", magic_error(magic_));
		magic_close(magic_);
		return;
	}

	const char* string = getenv("XDG_CONFIG_PATH");
	std::string configPath = string ? string : std::string();
	if(configPath.empty())
		configPath = "~/.config";

	configPath = FileSystem::realPath(configPath);
	if(configPath.back() != '/')
		configPath += '/';

	if(!FileSystem::isDirExists(configPath)) {
		LOG("Configuration directory '%s' is not exists.", configPath.c_str());
		magic_close(magic_);
		return;
	}

	configPath_ = configPath + PROJECT_NAME "/" PROJECT_NAME ".conf";

	LOG("Using configuration file: '%s'", configPath_.c_str());

	std::ifstream file(configPath_);
	if(file.is_open()) {
		std::string line;

		while(std::getline(file, line))
			processLine(line);
	}

	isInitialized_ = true;
}


LinkManager::~LinkManager()
{
	magic_close(magic_);
}


bool LinkManager::isInitialized() const
{
	return isInitialized_;
}


std::string LinkManager::pluginPath(const std::string& bridgePath) const
{
	auto it = pluginByBridge32_.find(bridgePath);
	if(it != pluginByBridge32_.end())
		return it->second;

	it = pluginByBridge64_.find(bridgePath);
	if(it != pluginByBridge64_.end())
		return it->second;

	return std::string();
}


LinkManager::Arch LinkManager::pluginArch(const std::string& bridgePath) const
{
	if(pluginByBridge32_.find(bridgePath) != pluginByBridge32_.end())
		return kArch32;

	if(pluginByBridge64_.find(bridgePath) != pluginByBridge64_.end())
		return kArch64;

	return kArchUnknown;
}


bool LinkManager::bind(const std::string& bridgePath,
		const std::string& pluginPath)
{
	if(setLink(bridgePath, pluginPath)) {
		save();
		return true;
	}

	return false;
}


bool LinkManager::rebind(const std::string& bridgePath,
		const std::string& newPath)
{
	for(auto it : pluginByBridge32_) {
		if(it.first == bridgePath) {
			std::string pluginPath = it.second;
			pluginByBridge32_.erase(bridgePath);
			pluginByBridge32_[newPath] = pluginPath;
			save();
			return true;
		}
	}

	for(auto it : pluginByBridge64_) {
		if(it.first == bridgePath) {
			std::string pluginPath = it.second;
			pluginByBridge32_.erase(bridgePath);
			pluginByBridge64_[newPath] = pluginPath;
			save();
			return true;
		}
	}

	return false;
}


bool LinkManager::unbind(const std::string& bridgePath)
{
	if(pluginByBridge32_.erase(bridgePath) > 0) {
		save();
		return true;
	}

	return false;
}


std::vector<std::string> LinkManager::boundBridges() const
{
	std::vector<std::string> result;

	for(auto it : pluginByBridge32_)
		result.push_back(it.first);

	for(auto it : pluginByBridge64_)
		result.push_back(it.first);

	return result;
}


LinkManager::Arch LinkManager::getArchitecture(const std::string& path) const
{
	const char* buffer = magic_file(magic_, path.c_str());

	if(buffer) {
		std::string string = buffer;

		if(string.find("80386") != std::string::npos) {
			return kArch32;
		}
		else if(string.find("x86-64") != std::string::npos) {
			return kArch64;
		}

	}

	return kArchUnknown;
}


bool LinkManager::setLink(const std::string& bridgePath,
		const std::string& pluginPath)
{
	Arch bridgeArch = getArchitecture(bridgePath);
	Arch pluginArch = getArchitecture(pluginPath);

	if(bridgeArch != pluginArch || bridgeArch == kArchUnknown)
		return false;

	if(bridgeArch == kArch32) {
		pluginByBridge32_[bridgePath] = pluginPath;
	}
	else {
		pluginByBridge64_[bridgePath] = pluginPath;
	}

	return true;
}


bool LinkManager::save()
{
	std::size_t pos = configPath_.rfind('/');
	if(pos != std::string::npos) {
		std::string dir = configPath_.substr(0, pos);
		if(!FileSystem::makePath(dir))
			return false;
	}

	std::ofstream file(configPath_, std::ios::out | std::ios::trunc);

	if(!file.is_open())
		return false;

	for(auto it : pluginByBridge32_)
		file << it.first << " = " << it.second << std::endl;

	for(auto it : pluginByBridge64_)
		file << it.first << " = " << it.second << std::endl;

	return true;
}


bool LinkManager::processLine(const std::string& line)
{
	std::size_t delimiter = line.find('=');
	if(delimiter == std::string::npos)
		return false;

	std::string bridgePath = trimString(line.substr(0, delimiter));
	std::string pluginPath = trimString(line.substr(delimiter + 1));

	return setLink(bridgePath, pluginPath);
}


std::string LinkManager::trimString(const std::string& string)
{
	std::size_t first = string.find_first_not_of(" \t");
	std::size_t last = string.find_last_not_of(" \t");

	return string.substr(first, last - first + 1);
}


} // namespace Airwave

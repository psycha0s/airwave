#include "linkmanager.h"

#include <fstream>
#include "config.h"
#include "filesystem.h"
#include "logger.h"


namespace Airwave {


LinkManager::LinkManager() :
	isInitialized_(false)
{
	const char* string = getenv("XDG_CONFIG_PATH");
	std::string configPath = string ? string : std::string();
	if(configPath.empty())
		configPath = "~/.config";

	configPath = FileSystem::realPath(configPath);
	if(configPath.back() != '/')
		configPath += '/';

	if(!FileSystem::isDirExists(configPath)) {
		LOG("Configuration directory '%s' is not exists.", configPath.c_str());
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


bool LinkManager::isInitialized() const
{
	return isInitialized_;
}


std::string LinkManager::pluginPath(const std::string& bridgePath) const
{
	auto it = pluginByBridge_.find(bridgePath);
	if(it != pluginByBridge_.end())
		return it->second;

	return std::string();
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
	for(auto it : pluginByBridge_) {
		if(it.first == bridgePath) {
			std::string pluginPath = it.second;
			pluginByBridge_.erase(bridgePath);
			pluginByBridge_[newPath] = pluginPath;
			save();
			return true;
		}
	}

	return false;
}


bool LinkManager::unbind(const std::string& bridgePath)
{
	if(pluginByBridge_.erase(bridgePath) > 0) {
		save();
		return true;
	}

	return false;
}


std::vector<std::string> LinkManager::boundBridges() const
{
	std::vector<std::string> result;

	for(auto it : pluginByBridge_)
		result.push_back(it.first);

	return result;
}


bool LinkManager::setLink(const std::string& bridgePath,
		const std::string& pluginPath)
{
	pluginByBridge_[bridgePath] = pluginPath;
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

	for(auto it : pluginByBridge_)
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

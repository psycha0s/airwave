#ifndef COMMON_LINKMANAGER_H
#define COMMON_LINKMANAGER_H

#include <map>
#include <string>
#include <vector>


namespace Airwave {


class LinkManager {
public:
	LinkManager();

	bool isInitialized() const;

	std::string pluginPath(const std::string& bridgePath) const;

	bool bind(const std::string& bridgePath, const std::string& pluginPath);
	bool rebind(const std::string& bridgePath, const std::string& newPath);
	bool unbind(const std::string& bridgePath);
	std::vector<std::string> boundBridges() const;

private:
	bool isInitialized_;
	std::string configPath_;
	std::map<std::string, std::string> pluginByBridge_;

	bool processLine(const std::string& line);
	bool setLink(const std::string& bridgePath, const std::string& pluginPath);
	bool save();

	static std::string trimString(const std::string& string);
};


} // namespace Airwave


#endif // COMMON_LINKMANAGER_H

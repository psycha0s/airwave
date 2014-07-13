#ifndef COMMON_LINKMANAGER_H
#define COMMON_LINKMANAGER_H

#include <map>
#include <string>
#include <vector>
#include <magic.h>


namespace Airwave {


class LinkManager {
public:
	enum Arch {
		kArchUnknown = 0,
		kArch32      = 32,
		kArch64      = 64
	};


	LinkManager();
	~LinkManager();

	bool isInitialized() const;

	std::string pluginPath(const std::string& bridgePath) const;
	Arch pluginArch(const std::string& bridgePath) const;

	bool bind(const std::string& bridgePath, const std::string& pluginPath);
	bool rebind(const std::string& bridgePath, const std::string& newPath);
	bool unbind(const std::string& bridgePath);
	std::vector<std::string> boundBridges() const;

private:
	bool isInitialized_;
	bool isChanged_;
	std::string configPath_;
	magic_t magic_;
	std::map<std::string, std::string> pluginByBridge32_;
	std::map<std::string, std::string> pluginByBridge64_;

	bool processLine(const std::string& line);
	Arch getArchitecture(const std::string& path) const;
	bool setLink(const std::string& bridgePath, const std::string& pluginPath);
	bool save();

	static std::string trimString(const std::string& string);
};


} // namespace Airwave


#endif // COMMON_LINKMANAGER_H

#ifndef COMMON_MODULEINFO_H
#define COMMON_MODULEINFO_H

#include <string>
#include <magic.h>


class ModuleInfo {
public:
	enum Arch {
		kArchUnknown = 0,
		kArch32      = 32,
		kArch64      = 64
	};

	static ModuleInfo* instance();

	ModuleInfo(const ModuleInfo&) = delete;
	ModuleInfo& operator=(const ModuleInfo&) = delete;

	~ModuleInfo();

	Arch getArch(const std::string& fileName) const;

private:
	bool isInitialized_;
	magic_t magic_;

	ModuleInfo();
};


#endif // COMMON_MODULEINFO_H

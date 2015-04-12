#ifndef CORE_MODULEINFO_H
#define CORE_MODULEINFO_H

#include <magic.h>


class QString;


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

	Arch getArch(const QString& fileName) const;

private:
	bool isInitialized_;
	magic_t magic_;

	ModuleInfo();
};


#endif // CORE_MODULEINFO_H

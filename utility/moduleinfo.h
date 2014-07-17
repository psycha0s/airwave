#ifndef UTILITY_MODULEINFO_H
#define UTILITY_MODULEINFO_H

#include <QString>
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

	Arch getArch(const QString& fileName) const;

private:
	bool isInitialized_;
	magic_t magic_;

	ModuleInfo();
};


#endif // UTILITY_MODULEINFO_H

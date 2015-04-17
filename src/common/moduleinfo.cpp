#include "moduleinfo.h"


ModuleInfo::ModuleInfo() :
	isInitialized_(false)
{
	magic_ = magic_open(MAGIC_NONE);
	if(!magic_) {
		return;
	}

	if(magic_load(magic_, nullptr) != 0) {
		magic_close(magic_);
		return;
	}

	isInitialized_ = true;
}


ModuleInfo::~ModuleInfo()
{
	magic_close(magic_);
}


ModuleInfo* ModuleInfo::instance()
{
	static ModuleInfo info;
	return &info;
}


ModuleInfo::Arch ModuleInfo::getArch(const std::string& fileName) const
{
	if(isInitialized_) {
		const char* buffer = magic_file(magic_, fileName.c_str());

		if(buffer) {
			std::string string = buffer;

			if(string.find("80386") != std::string::npos) {
				return kArch32;
			}
			else if(string.find("x86-64") != std::string::npos) {
				return kArch64;
			}
		}
	}

	return kArchUnknown;
}

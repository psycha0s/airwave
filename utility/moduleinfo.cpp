#include "moduleinfo.h"


ModuleInfo::ModuleInfo() :
	isInitialized_(false)
{
	magic_ = magic_open(MAGIC_NONE);
	if(!magic_) {
		qDebug("Unable to initialize libmagic.");
		return;
	}

	if(magic_load(magic_, nullptr) != 0) {
		qDebug("libmagic error: %s", magic_error(magic_));
		magic_close(magic_);
		return;
	}

	isInitialized_ = true;
}


ModuleInfo::~ModuleInfo()
{
	magic_close(magic_);
}


bool ModuleInfo::isInitialized() const
{
	return isInitialized_;
}


ModuleInfo::Arch ModuleInfo::getArch(const QString& fileName) const
{
	const char* buffer = magic_file(magic_, fileName.toAscii().data());

	if(buffer) {
		QString string = buffer;

		if(string.indexOf("80386") != -1) {
			return kArch32;
		}
		else if(string.indexOf("x86-64") != -1) {
			return kArch64;
		}
	}

	return kArchUnknown;
}

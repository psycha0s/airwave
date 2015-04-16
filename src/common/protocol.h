#ifndef COMMON_PROTOCOL_H
#define COMMON_PROTOCOL_H

#include "common/types.h"


namespace Airwave {


enum class Command {
	Response,
	Dispatch,
	GetParameter,
	SetParameter,
	ProcessSingle,
	ProcessDouble,
	HostInfo,
	PluginInfo,
	ShowWindow,
	GetDataBlock,
	SetDataBlock,
	AudioMaster
};


struct DataFrame {
	Command  command;
	i32      opcode;
	i32      index;
	intptr_t value;
	float    opt;

#ifdef __i386
	// The size of a pointer on the x86_64 in two times larger than on the x86. Because
	// of this, we should use the 32 bits of padding on the x86.
	void*    padding;
#endif

	u8       data[];
} __attribute__((packed));


struct PluginInfo {
	i32 flags;
	i32 programCount;
	i32 paramCount;
	i32 inputCount;
	i32 outputCount;
	i32 uniqueId;
	i32 version;
} __attribute__((packed));


} // namespace Airwave


#endif // COMMON_PROTOCOL_H

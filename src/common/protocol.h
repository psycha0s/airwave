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
	Command command;
	i32     opcode;
	i32     index;
	i64     value;	// The 64-bit value is used here to avoid 64->32 bridging issues
	float   opt;
	u8      data[];
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

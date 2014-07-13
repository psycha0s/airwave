#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "types.h"


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
	AudioMaster
};


struct DataFrame {
	Command  command;
	int32_t  opcode;
	int32_t  index;
	intptr_t value;
	float    opt;
	uint8_t  data[];
} __attribute__((packed));


struct PluginInfo {
	int32_t flags;
	int32_t programCount;
	int32_t paramCount;
	int32_t inputCount;
	int32_t outputCount;
	int32_t uniqueId;
	int32_t version;
} __attribute__((packed));


} // namespace Airwave


#endif // PROTOCOL_H

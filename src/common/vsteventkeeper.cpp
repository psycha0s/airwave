#include "vsteventkeeper.h"

#include <algorithm>
#include <cstdint>


namespace Airwave {


VstEventKeeper::VstEventKeeper() :
	events_(nullptr),
	data_(nullptr)
{
}


VstEventKeeper::~VstEventKeeper()
{
	delete [] events_;
}


void VstEventKeeper::reload(int count, const VstEvent events[])
{
	if(!events_ || events_->numEvents < count) {
		delete [] events_;

		int extraCount = std::max(count - 2, 0);

		size_t size = sizeof(VstEvents) + extraCount * sizeof(VstEvent*) +
				count * sizeof(VstEvent);

		uint8_t* buffer = new uint8_t[size];
		events_ = reinterpret_cast<VstEvents*>(buffer);

		size_t offset = sizeof(VstEvents) + extraCount * sizeof(VstEvent*);
		data_ = reinterpret_cast<VstEvent*>(buffer + offset);
	}

	events_->numEvents = count;
	events_->reserved = 0;

	for(int i = 0; i < count; ++i) {
		data_[i] = events[i];
		events_->events[i] = &data_[i];
	}
}


VstEvents* VstEventKeeper::events()
{
	return events_;
}


} // namespace Airwave

#ifndef COMMON_VSTEVENTSKEEPER_H
#define COMMON_VSTEVENTSKEEPER_H

#include <aeffectx.h>


namespace Airwave {


class VstEventKeeper {
public:
	VstEventKeeper();
	~VstEventKeeper();

	void reload(int count, const VstEvent events[]);
	VstEvents* events();

private:
	VstEvents* events_;
	VstEvent* data_;
};


} // namespace Airwave


#endif // COMMON_VSTEVENTSKEEPER_H

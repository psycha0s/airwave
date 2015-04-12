#ifndef COMMON_EVENTSIGNAL_H
#define COMMON_EVENTSIGNAL_H

#include <semaphore.h>


namespace Airwave {


class EventSignal {
public:
	EventSignal();
	~EventSignal();

	bool wait(int msecs = -1);
	void post();

private:
	sem_t sem_;
};


} // namespace Airwave


#endif // COMMON_EVENTSIGNAL_H

#include "eventsignal.h"

#include <cerrno>
#include <cstring>
#include <ctime>
#include "common/logger.h"


namespace Airwave {


EventSignal::EventSignal()
{
	sem_init(&sem_, 1, 0);
}


EventSignal::~EventSignal()
{
	sem_destroy(&sem_);
}


bool EventSignal::wait(int msecs)
{
	if(msecs < 0) {
		if(sem_wait(&sem_) != 0) {
			ERROR("sem_wait() call failed: %d", errno);
			return false;
		}
	}
	else {
		int seconds = msecs / 1000;
		msecs %= 1000;

		timespec tm;
		clock_gettime(CLOCK_REALTIME, &tm);
		tm.tv_sec += seconds;
		tm.tv_nsec += msecs * 1000000;

		if(tm.tv_nsec >= 1000000000L) {
			tm.tv_sec++;
			tm.tv_nsec -= 1000000000L;
		}

		if(sem_timedwait(&sem_, &tm) != 0) {
			if(errno != ETIMEDOUT)
				ERROR("sem_timedwait() call failed: %d", errno);

			return false;
		}
	}

	return true;
}


void EventSignal::post()
{
	if(sem_post(&sem_) != 0)
		ERROR("sem_post() call failed: %s", strerror(errno));
}


} // namespace Airwave

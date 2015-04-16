#include "event.h"

#include <errno.h>
#include <syscall.h>
#include <time.h>
#include <unistd.h>
#include <linux/futex.h>


#define futex_wait(futex, count, timeout) \
		(syscall(SYS_futex, futex, FUTEX_WAIT, count, timeout, nullptr, 0) == 0)

#define futex_post(futex, count) \
		(syscall(SYS_futex, futex, FUTEX_WAKE, count, nullptr, nullptr, 0) == 0)


Event::Event() :
	count_(0)
{
}


bool Event::wait(int msecs)
{
	timespec* timeout = nullptr;
	timespec tm;

	if(msecs >= 0) {
		int seconds = msecs / 1000;
		msecs %= 1000;

		tm.tv_sec  = seconds;
		tm.tv_nsec = msecs * 1000000;

		timeout = &tm;
	}

	while(count_ == 0) {
		if(!futex_wait(&count_, 0, timeout) && errno != EWOULDBLOCK)
			return false;
	}

	count_--;
	return true;
}


void Event::post()
{
	count_++;
	futex_post(&count_, 1);
}

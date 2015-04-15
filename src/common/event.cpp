#include "event.h"

#include <syscall.h>
#include <time.h>
#include <unistd.h>
#include <linux/futex.h>


#define futex_wait(count, timeout) \
		(syscall(SYS_futex, count, FUTEX_WAIT, 0, timeout, nullptr, 0) == 0)

#define futex_post(count) \
		(syscall(SYS_futex, count, FUTEX_WAKE, 1, nullptr, nullptr, 0) == 0)


Event::Event() :
	count_(0)
{
}


bool Event::wait(int msecs)
{
	bool result;

	if(msecs < 0) {
		result = futex_wait(&count_, nullptr);
	}
	else {
		int seconds = msecs / 1000;
		msecs %= 1000;

		timespec tm;
		tm.tv_sec  = seconds;
		tm.tv_nsec = msecs * 1000000;

		result = futex_wait(&count_, &tm);
	}

	if(result)
		count_--;

	return result;
}


void Event::post()
{
	count_++;
	futex_post(&count_);
}

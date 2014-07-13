#ifndef COMMON_LOGGER_H_
#define COMMON_LOGGER_H_

#include <ctime>


#ifdef DEBUG_BUILD

#define LOG(format, ...)                                                       \
	do {                                                                       \
		timespec tm;                                                           \
		clock_gettime(CLOCK_REALTIME, &tm);                                    \
		debug("%lu.%09lu: " format "\n", tm.tv_sec, tm.tv_nsec, ##__VA_ARGS__);\
	} while(0)


void debug(const char* format, ...);

#else

#define LOG(format, ...) do { } while(0)

#endif


#endif // COMMON_LOGGER_H_

#include "logger.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <linux/un.h>
#include <sys/socket.h>


#ifdef DEBUG_BUILD

#ifndef LOG_SOCK_PATH
#define LOG_SOCK_PATH "/tmp/log.sock"
#endif // LOG_SOCK_PATH


struct Log {
	int fd_;

	Log()
	{
/*		fd_ = socket(PF_UNIX, SOCK_STREAM, 0);
		if(fd_ < 0)
			return;

		sockaddr_un address;
		memset(&address, 0, sizeof(sockaddr_un));
		address.sun_family = AF_UNIX;
		snprintf(address.sun_path, UNIX_PATH_MAX, LOG_SOCK_PATH);

		if(connect(fd_, reinterpret_cast<sockaddr*>(&address),
				sizeof(sockaddr_un)) != 0) {
			close(fd_);
			fd_ = -1;
		}*/
	}


	~Log()
	{
/*		if(fd_ >= 0)
			close(fd_);*/
	}
};


static Log logger;


void debug(const char* format, ...)
{
	va_list args;
	va_start(args, format);

//	vdprintf(logger.fd_, format, args);
	vfprintf(stdout, format, args);

	va_end(args);

//	fsync(logger.fd_);
	fflush(stdout);
}


#endif

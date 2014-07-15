#include "logger.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <linux/un.h>
#include <sys/socket.h>
#include <vector>
#include "types.h"


static int fd = -1;
static std::string id;
static std::vector<char> buffer;


bool loggerInit(const std::string& senderId)
{
	fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(fd < 0)
		return false;

	sockaddr_un address;
	memset(&address, 0, sizeof(sockaddr_un));
	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, UNIX_PATH_MAX, LOG_SOCKET_PATH);

	if(connect(fd, reinterpret_cast<sockaddr*>(&address),
			sizeof(sockaddr_un)) != 0) {
		close(fd);
		fd = -1;
		return false;
	}

	id = senderId;

	// FIXME
	buffer.resize(1024);
	return true;
}



void loggerFree()
{
	if(fd >= 0) {
		close(fd);
		id.clear();
	}
}


void loggerLogMessage(const char* format, ...)
{
	timespec tm;                                                           \
	clock_gettime(CLOCK_REALTIME, &tm);                                    \

	uint64_t* timeStamp = reinterpret_cast<uint64_t*>(buffer.data());
	*timeStamp = (static_cast<uint64_t>(tm.tv_sec) << 32) + tm.tv_nsec;

	char* output = buffer.data() + sizeof(uint64_t);
	output = std::copy(id.begin(), id.end(), output);
	*output = '\x01';
	++output;

	size_t count = output - buffer.data();

	va_list args;
	va_start(args, format);

	count += std::vsnprintf(output, buffer.size() - count, format, args) + 1;

	va_end(args);

	send(fd, buffer.data(), count, 0);
	fsync(fd);
}


void loggerSetSenderId(const std::string& senderId)
{
	id = senderId;
}

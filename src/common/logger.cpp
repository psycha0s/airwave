#include "logger.h"

#include <cstdarg>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <linux/un.h>
#include <sys/socket.h>
#include <vector>
#include "common/types.h"


namespace Airwave {


static int fd = -1;
static std::string id;
static std::vector<char> buffer;
static LogLevel defaultLevel = LogLevel::kDebug;


bool loggerInit(const std::string& socketPath, const std::string& senderId)
{
	fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if(fd < 0)
		return false;

	sockaddr_un address;
	memset(&address, 0, sizeof(sockaddr_un));
	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, UNIX_PATH_MAX, socketPath.c_str());

	if(connect(fd, reinterpret_cast<sockaddr*>(&address),
			sizeof(sockaddr_un)) != 0) {
		close(fd);
		fd = -1;
		return false;
	}

	id = senderId;

	// NOTE The buffer size is hardcoded, increase it if necessary.
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


LogLevel loggerLogLevel()
{
	return defaultLevel;
}


void loggerSetLogLevel(LogLevel level)
{
	defaultLevel = level;
}


std::string loggerSenderId()
{
	return id;
}


void loggerSetSenderId(const std::string& senderId)
{
	id = senderId;
}


void loggerMessage(LogLevel level, const char* format, ...)
{
	if(level > defaultLevel)
		return;

	if(fd == -1)
		return;

	timespec tm;
	clock_gettime(CLOCK_REALTIME, &tm);

	u64* timestamp = reinterpret_cast<u64*>(buffer.data());
	*timestamp = (static_cast<u64>(tm.tv_sec) << 32) + tm.tv_nsec;

	char* output = buffer.data() + sizeof(u64);
	output = std::copy(id.begin(), id.end(), output);
	*output = '\x01';
	++output;

	uint count = output - buffer.data();

	va_list args;
	va_start(args, format);

	count += std::vsnprintf(output, buffer.size() - count, format, args) + 1;

	va_end(args);

	send(fd, buffer.data(), count, 0);
}


} // namespace Airwave

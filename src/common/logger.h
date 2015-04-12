#ifndef COMMON_LOGGER_H
#define COMMON_LOGGER_H

#include <string>


#define FLOOD(format, ...) \
		Airwave::loggerMessage(Airwave::LogLevel::kFlood, format, ##__VA_ARGS__)

#define DEBUG(format, ...) \
		Airwave::loggerMessage(Airwave::LogLevel::kDebug, format, ##__VA_ARGS__)

#define TRACE(format, ...) \
		Airwave::loggerMessage(Airwave::LogLevel::kTrace, format, ##__VA_ARGS__)

// Workaround for wingdi.h
#ifdef ERROR
#undef ERROR
#endif

#define ERROR(format, ...) \
		Airwave::loggerMessage(Airwave::LogLevel::kError, format, ##__VA_ARGS__)

namespace Airwave {


enum class LogLevel {
	kDefault = -1,
	kQuiet,
	kError,
	kTrace,
	kDebug,
	kFlood
};


bool loggerInit(const std::string& socketPath, const std::string& senderId);
void loggerFree();
LogLevel loggerLogLevel();
void loggerSetLogLevel(LogLevel level);
std::string loggerSenderId();
void loggerSetSenderId(const std::string& senderId);
void loggerMessage(LogLevel level, const char* format, ...);


} // namespace Airwave


#endif // COMMON_LOGGER_H

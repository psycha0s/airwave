#ifndef COMMON_LOGGER_H
#define COMMON_LOGGER_H

#include <string>
#include "config.h"
#include "types.h"


#ifdef ENABLE_LOGGER

#define LOG(format, ...) loggerLogMessage(format, ##__VA_ARGS__)

#else

#define LOG(format, ...)

#endif


bool loggerInit(const std::string& senderId);
void loggerFree();
void loggerLogMessage(const char* format, ...);
void loggerSetSenderId(const std::string& senderId);



#endif // COMMON_LOGGER_H

#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} log_level_t;

/* Set the minimum log level — messages below this level are discarded */
void logger_init(log_level_t min_level);

/* Core log function — use the macros below instead of calling directly */
void logger_log(log_level_t level, const char* file, int line, const char* fmt, ...);

/* Convenience macros that auto-inject source file and line number */
#define LOG_DEBUG(fmt, ...) logger_log(LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  logger_log(LOG_INFO,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  logger_log(LOG_WARN,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) logger_log(LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif

#include "../include/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

static log_level_t g_min_level = LOG_DEBUG;
static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char* level_strings[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR"
};

static const char* level_colors[] = {
    "\033[36m",   /* DEBUG — cyan */
    "\033[32m",   /* INFO  — green */
    "\033[33m",   /* WARN  — yellow */
    "\033[31m",   /* ERROR — red */
};

#define COLOR_RESET "\033[0m"

void logger_init(log_level_t min_level) {
    g_min_level = min_level;
}

/* Extract just the filename from a full path for cleaner output */
static const char* basename_from_path(const char* path) {
    const char* slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

void logger_log(log_level_t level, const char* file, int line, const char* fmt, ...) {
    if (level < g_min_level) return;

    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm tm_info;
    localtime_r(&tv.tv_sec, &tm_info);

    char time_buf[32];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm_info);

    pthread_mutex_lock(&g_log_mutex);

    fprintf(stderr, "%s%s.%03ld [%-5s] [tid:%lu] %s:%d — ",
            level_colors[level],
            time_buf,
            tv.tv_usec / 1000,
            level_strings[level],
            (unsigned long)pthread_self(),
            basename_from_path(file),
            line);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "%s\n", COLOR_RESET);
    fflush(stderr);

    pthread_mutex_unlock(&g_log_mutex);
}

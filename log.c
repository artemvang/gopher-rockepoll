#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <err.h>
#include <time.h>

#include "log.h"


#define TIMESTAMP_SIZE 64
#define TIMESTAMP_FORMAT "[%a, %d/%b/%Y %H:%M:%S GMT] "


static int quiet = 0;


void
init_logger(int quiet_mode)
{
    quiet = quiet_mode;
}


inline void
log_log(const time_t *time, const char *format, ...)
{
    struct tm *tm;
    char timestamp[TIMESTAMP_SIZE] = {0};
    va_list va;

    if (!quiet) {
        tm = gmtime(time);
        strftime(timestamp, sizeof(timestamp), TIMESTAMP_FORMAT, tm);
        printf(timestamp);

        va_start(va, format);
        vprintf(format, va);
        va_end(va);
    }
}
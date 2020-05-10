
#ifndef LOG_H
#define LOG_H

#include <time.h>


#ifndef __printflike
# ifdef __GNUC__
/* [->] borrowed from FreeBSD's src/sys/sys/cdefs.h,v 1.102.2.2.2.1 */
#  define __printflike(fmtarg, firstvararg) \
             __attribute__((__format__(__printf__, fmtarg, firstvararg)))
/* [<-] */
# else
#  define __printflike(fmtarg, firstvararg)
# endif
#endif


void init_logger(int quiet_mode);
void log_log(const time_t *time, const char *format, ...) __printflike(2, 3);


#endif
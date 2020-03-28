#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include "logger.h"

/*
 * Return number of milliseconds elapsed since boot
 */
long
time_in_msecs()
{
        struct timespec ts;
        (void)clock_gettime(CLOCK_BOOTTIME, &ts);
        return (ts.tv_sec * 1000 + ts.tv_nsec/1000);
}

/*
 * Log the error message to standard error and exit.
 */
void fatal_error(const char *errmsg)
{
        logger(ERROR, errmsg);
        perror(errmsg);
        exit(EXIT_FAILURE);
}

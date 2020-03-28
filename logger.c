#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "logger.h"

const char *loglevel_strings[] = {
	FOREACH_LOGLEVEL(ENUM_STR)
};

static loglevel_t log_level = DEFAULT_LOGLEVEL;

static int
log_level_is_valid(loglevel_t level)
{
	return (level >= log_level);
}

void
set_log_level(loglevel_t level)
{
	log_level = level;
}

const char *
loglevel_to_str(loglevel_t level)
{
	return loglevel_strings[level];
}

/*
 * A simple logging utility function to log messages to standard
 * output. Can add file support later on.
 * Check logger.h for support log levels.
 */
void
logger(loglevel_t level, const char *fmt, ...)
{
	va_list args;
	char buffer[MAX_LOGMSG_LEN];
	time_t cur_time;
	char time_str[32];

	if (!log_level_is_valid(level)) {
		return;
	}

	/*
	 * Stringify the arguments.
	 */
	va_start(args, fmt);
	(void)vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	/*
	 * Get the current time as string
	 */
	cur_time = time(NULL);
	struct tm *local_time = localtime(&cur_time);
	strftime(time_str, sizeof(time_str), "%c", local_time);

	/*
	 * Sending to stdout only for now.
	 */
	fprintf(stdout, "[%s],%s,%s\n", time_str, loglevel_to_str(level), buffer);
}

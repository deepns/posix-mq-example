#ifndef _LOG_H_
#define _LOG_H_

#define FOREACH_LOGLEVEL(LOGLEVEL) \
	LOGLEVEL(DEBUG), \
	LOGLEVEL(INFO), \
	LOGLEVEL(WARNING), \
	LOGLEVEL(ERROR) \

#define ENUM(e) e
#define ENUM_STR(e) #e

enum loglevel {
	FOREACH_LOGLEVEL(ENUM)
};

typedef enum loglevel loglevel_t;

#define DEFAULT_LOGLEVEL DEBUG
#define MAX_LOGMSG_LEN 512

const char * loglevel_to_str(loglevel_t level);
void set_log_level(loglevel_t level);
void logger(loglevel_t level, const char *fmt, ...);

#endif

#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>

#include "app_msg.h"
#include "logger.h"


// TODO
// 1. [DONE] Move msg definitions into a sep .h/.c files
// 2. [DONE] change ap_send_msg and ap_recv_msg defintions to take the threadinfo (or mqd) as argument instead of thread num.
// 3. [DONE] add a simple logger interface that takes a severity and message with variable number of arguments
// 4. [DONE] In appd, change the logger thread to something else. say notifier?
// 5. [NOT_NEEDED] In ap_recv_msg, send a response (non-blocking?) to the sender. What is the use of ack?
// 6. Change health_checker and logger to include an infinite loop. Change the interval time of notifier
// 7. [DONE] Use EXIT_SUCESS and EXIT_FAILURE from stdlib instead of manual definition.
// 8. [DONE] Get qtime in milliseconds instead of seconds (using gettimeofday() or clock_time()?)
// 9. [DONE] Add a makefile
// 10. Do something about fatal_error. It doesn't look right.
// 11. [DONE] Change ap_recv_msg to return the malloc'd message.
// 12. Add comments to all files.
// 13. Delete the message queue in the cleanup()
// 14. Test for mem leaks
// 15. Retab to 4 spaces

void fatal_error(const char *errmsg)
{
	logger(ERROR, errmsg);
	perror(errmsg);
	exit(EXIT_FAILURE);
}

void* health_checker(void *arg);
void* notifier(void *arg);

struct appdata appd = {
	.health_checker = {
		.id = 0,
		.num = 0,
		.name = "health_checker",
		.mqd = -1,
		.start = &health_checker
	},
	.notifier = {
		.id = 0,
		.num = 1,
		.name = "notifier",
		.mqd = -1,
		.start = &notifier 
	}
};

long
time_in_msecs()
{
	struct timespec ts;
	// ignore errors returned from clock_gettime for now
	(void)clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_nsec/1000);
}


void* health_checker(void *arg)
{
	logger(DEBUG, "In health_checker()");
	thread_info_t *tinfo = (thread_info_t *)arg;
	appd_msg_t msg;

	msg.type = NET_STATUS_UP;
	msg.sender_id = tinfo->id;
	msg.qtime = time_in_msecs();
	msg.payload = NULL;

	for (int i=0; i<100; i++) {
		msg.qtime = time_in_msecs();
		logger(DEBUG, "Sending message to notifier, msecs=%lu\n", msg.qtime);
		ap_send_msg(&appd.notifier, &msg);
		sleep(2);
	}

	return NULL;
}

void* notifier(void *arg)
{
	logger(DEBUG, "In notifier()");
	appd_msg_t *msg;
	long start_time = time_in_msecs();
	long elapsed_time = time_in_msecs();
	long msg_recv_time;

	for (;;) {
		msg = ap_recv_msg(&appd.notifier);
		msg_recv_time = time_in_msecs();
		logger(DEBUG, "Received message from thread-id %lu,"
				" qtime=%lu, transit time=%lu ms\n",
				msg->sender_id,
				msg->qtime,
				msg_recv_time - msg->qtime);
		free(msg);
		
		elapsed_time = time_in_msecs() - start_time;
		/*
		 * Sleep for sometime every 30 secs
		 */
		if (elapsed_time >= 10000) {
			logger(DEBUG, "Notifier going to sleep");
			sleep(5);
			start_time = time_in_msecs();
		}
	}
	return NULL;
}

#define MSG_QUEUE_SIZE 10

static mqd_t
create_message_queue(const char *name)
{
	struct mq_attr attr;
	char *mq_name;
	int mq_name_len = strlen(name) + strlen("/app-") + 1;
	mqd_t qd = -1;

	/*
	 * Set the basic attributes of the message queue.
	 * mq_flags and mq_curmsgs are not relevant to mq_open().
	 */
	attr.mq_flags = 0;
	attr.mq_maxmsg = MSG_QUEUE_SIZE;
	attr.mq_msgsize = sizeof(appd_msg_t);
	attr.mq_curmsgs = 0;

	mq_name = calloc(mq_name_len, sizeof(char));
	snprintf(mq_name, mq_name_len, "/app-%s", name);

	logger(DEBUG, "Creating %s mqueue", mq_name);
	qd = mq_open((const char *)mq_name,
		O_CREAT | O_RDWR,
		S_IRUSR | S_IWUSR,
		&attr);
	free(mq_name);

	if (qd == -1) {
		fatal_error("Failed to create message queue");
	}

	return qd;
}

void init()
{
	set_log_level(DEBUG);
	appd.health_checker.mqd = create_message_queue(appd.health_checker.name);
	appd.notifier.mqd = create_message_queue(appd.notifier.name);
}

void start_threads()
{
	int err;
	err = pthread_create(&appd.health_checker.id,
					NULL,
					appd.health_checker.start,
					(void *)&appd.health_checker);
	if (err) {
		fatal_error("failed to create health_checker thread");
	}

	err = pthread_create(&appd.notifier.id,
					NULL,
					appd.notifier.start,
					(void *)&appd.notifier);
	if (err) {
		fatal_error("failed to create notifier thread");
	}
}

void wait_for_all_threads()
{
	int err;
	err = pthread_join(appd.health_checker.id, NULL);
	if (err) {
		fatal_error("failed to join health_checker thread");
	}

	err = pthread_join(appd.notifier.id, NULL);
	if (err) {
		fatal_error("failed to join notifier thread");
	}
}

void cleanup()
{
	mq_close(appd.health_checker.id);
	mq_close(appd.notifier.id);
}

int main(void)
{
	init();
	start_threads();
	wait_for_all_threads();
	cleanup();
	return 0;
}


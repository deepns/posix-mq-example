#ifndef _APPMSG_H_
#define _APPMSG_H_

#include <mqueue.h>

typedef enum msg_type {
        INVALID,
        NET_STATUS_UP, // some dummy messages
        NET_STATUS_DOWN
} msg_type_t;

typedef struct appd_msg {
        msg_type_t type;
        long qtime;
        int sender_id;
        void* payload;
} appd_msg_t;

typedef struct thread_info {
	pthread_t id;
	int num;
	const char* name;
	mqd_t mqd;
	void *(*start) (void *);
} thread_info_t;

struct appdata {
	thread_info_t health_checker;
	thread_info_t notifier;
};

appd_msg_t*  ap_recv_msg(thread_info_t *src);
void ap_send_msg(thread_info_t *dest, appd_msg_t *msg);

#endif

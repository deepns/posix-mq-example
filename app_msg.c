#include <time.h>
#include <stdlib.h>
#include "app_msg.h"
#include "logger.h"

void
ap_send_msg(thread_info_t *dest, appd_msg_t *msg)
{
	int status;
	if (!dest || dest->mqd == -1) {
		logger(ERROR, "%s, destination mq not available");
		return;
	}
	status = mq_send(dest->mqd, (const char *)msg, sizeof(appd_msg_t), 0); 
	if (status == -1) {
		logger(ERROR, "Unable to send message");
	}   
}

appd_msg_t *
ap_recv_msg(thread_info_t *src) 
{  
	appd_msg_t *msg = NULL;
	int num_bytes_received;

	if (!src || src->mqd == -1) {
		logger(ERROR, "%s, src mq not available", __func__);
		return NULL;
	}

	msg = (appd_msg_t *)calloc(sizeof(appd_msg_t), 1);
	if (msg == NULL) {
		logger(ERROR, "Failed to allocate memory for app_msg in %s", __func__);
		exit(EXIT_FAILURE);
	} 

	num_bytes_received = mq_receive(src->mqd, (char *)msg, sizeof(appd_msg_t), NULL);
	if (num_bytes_received == -1) {
		logger(ERROR, "Failed to receive message");
	}

	return msg;
}

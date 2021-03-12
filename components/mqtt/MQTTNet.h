#ifndef H_MQTTNET_H_
#define H_MQTTNET_H_

#include "ezdev_sdk_kernel_struct.h"
#include "mkernel_internal_error.h"

typedef struct MQTTNetwork
{
	ezdev_sdk_net_work my_socket;
	int (*mqttread) (struct MQTTNetwork*, unsigned char*, int, int);
	int (*mqttwrite) (struct MQTTNetwork*, unsigned char*, int, int);
} Network;

void MQTTNetInit(Network* net_work);
mkernel_internal_error MQTTNetConnect(Network* net_work, char* ip, int port);
void MQTTNetDisconnect(Network* net_work);
void MQTTNetFini(Network* net_work);

mkernel_internal_error MQTTNetGetLastError();
void MQTTNetSetLastError(mkernel_internal_error code);

typedef struct Timer
{
	ezdev_sdk_time end_time;
} Timer;

void TimerInit(Timer* assign_timer);
char TimerIsExpiredByDiff(Timer* assign_timer, unsigned int time_ms);
char TimerIsExpired(Timer* assign_timer);
void TimerCountdownMS(Timer* assign_timer, unsigned int time_count);
void TimerCountdown(Timer* assign_timer, unsigned int time_count);
int TimerLeftMS(Timer* assign_timer);
void TimerFini(Timer* assign_timer);


#endif
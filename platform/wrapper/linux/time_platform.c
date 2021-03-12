#include "time_platform_wrapper.h"
#include <time.h>
#include "ezdev_sdk_kernel_struct.h"

#define TIMESPEC_THOUSAND	1000
#define TIMESPEC_MILLION	1000000
#define TIMESPEC_BILLION	1000000000

# define Platform_Timespec_Add(a, b, result)						      \
	do {									      \
		(result)->tv_sec = (a)->tv_sec + (b)->tv_sec;			      \
		(result)->tv_nsec = (a)->tv_nsec + (b)->tv_nsec;			      \
		if ((result)->tv_nsec >= TIMESPEC_BILLION)					      \
		{									      \
			++(result)->tv_sec;						      \
			(result)->tv_nsec -= TIMESPEC_BILLION;					      \
		}									      \
	} while (0)
# define Platform_Timespec_Sub(a, b, result)						      \
	do {									      \
		(result)->tv_sec = (a)->tv_sec - (b)->tv_sec;			      \
		(result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec;			      \
		if ((result)->tv_nsec < 0) {					      \
			--(result)->tv_sec;						      \
			(result)->tv_nsec += TIMESPEC_BILLION;					      \
		}									      \
	} while (0)

ezdev_sdk_time Platform_TimerCreater()
{
	linux_time* linuxtime = NULL;
	linuxtime = (linux_time*)malloc(sizeof(linux_time));
	if (linuxtime == NULL)
	{
		return NULL;
	}

	linuxtime->time_record = (struct timespec){0, 0};
	return linuxtime;
}

char Platform_TimeIsExpired_Bydiff(ezdev_sdk_time sdktime, EZDEV_SDK_UINT32 time_ms)
{
	struct timespec now, res;
	linux_time* linuxtime = (linux_time*)sdktime;
	if (linuxtime == NULL)
	{
		return (char)1;
	}

	clock_gettime(CLOCK_MONOTONIC, &now);
	Platform_Timespec_Sub(&now, &linuxtime->time_record, &res);


// 	char now_time[16];
// 	time_t t = time(0);
// 	strftime(now_time, 16, "%m-%d %H:%M:%S", localtime(&t));
// 
//  	printf("[%s][0][0]pre time:%d:%d  now time:%d:%d\n restime:%d:%d diff:%d \n",
//  		now_time, linuxtime->time_record.tv_sec, linuxtime->time_record.tv_usec,
//  		now.tv_sec, now.tv_usec, res.tv_sec, res.tv_usec, time_ms);

	if (res.tv_sec < 0)
	{
		return (char)0;
	}
	else if (res.tv_sec == 0)
	{
		if ((res.tv_nsec/TIMESPEC_MILLION) > time_ms)
		{
			return (char)1;
		}
		else
		{
			return (char)0;
		}
	}
	else
	{
		if ( (res.tv_sec*TIMESPEC_THOUSAND + res.tv_nsec/TIMESPEC_MILLION) > time_ms )
		{
			return (char)1;
		}
		else
		{
			return (char)0;
		}
	}
}

char Platform_TimerIsExpired(ezdev_sdk_time sdktime)
{
	struct timespec now, res;
	linux_time* linuxtime = (linux_time*)sdktime;
	if (linuxtime == NULL)
	{
		return (char)1;
	}

	clock_gettime(CLOCK_MONOTONIC, &now);
	Platform_Timespec_Sub(&linuxtime->time_record, &now, &res);		
	return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_nsec <= 0);
}


void Platform_TimerCountdownMS(ezdev_sdk_time sdktime, unsigned int timeout)
{
	struct timespec now;
	struct timespec interval = {timeout / TIMESPEC_THOUSAND, (timeout % TIMESPEC_THOUSAND) * TIMESPEC_MILLION};
	linux_time* linuxtime = (linux_time*)sdktime;
	if (linuxtime == NULL)
	{
		return;
	}

	clock_gettime(CLOCK_MONOTONIC, &now);
	Platform_Timespec_Add(&now, &interval, &linuxtime->time_record);
}


void Platform_TimerCountdown(ezdev_sdk_time sdktime, unsigned int timeout)
{
	struct timespec now;
	struct timespec interval = {timeout, 0};
	linux_time* linuxtime = (linux_time*)sdktime;
	if (linuxtime == NULL)
	{
		return;
	}

	clock_gettime(CLOCK_MONOTONIC, &now);
	Platform_Timespec_Add(&now, &interval, &linuxtime->time_record);
}

EZDEV_SDK_UINT32 Platform_TimerLeftMS(ezdev_sdk_time sdktime)
{
	struct timespec now, res;
	linux_time* linuxtime = (linux_time*)sdktime;
	if (linuxtime == NULL)
	{
		return 0;
	}

	clock_gettime(CLOCK_MONOTONIC, &now);
	Platform_Timespec_Sub(&linuxtime->time_record, &now, &res);
	//printf("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000);
	return (res.tv_sec < 0) ? 0 : res.tv_sec * TIMESPEC_THOUSAND + res.tv_nsec / TIMESPEC_MILLION;
}

void Platform_TimeDestroy(ezdev_sdk_time sdktime)
{
	linux_time* linuxtime = (linux_time*)sdktime;
	if (linuxtime == NULL)
	{
		return;
	}
	
	free(linuxtime);
	linuxtime = NULL;
}

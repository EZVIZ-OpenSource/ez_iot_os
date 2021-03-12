/*******************************************************************************
 * Copyright © 2017-2021 Ezviz Inc.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *******************************************************************************/

#include "time_platform_wrapper.h"
#include <time.h>
#include "ezdev_sdk_kernel_struct.h"

/** 
 *  \brief		创建时间对象
 *  \method		Platform_TimerCreater
 *  \return 	成功返回时间对象 失败返回NULL
 */
ezdev_sdk_time Platform_TimerCreater()
{
	win_time* wintime = NULL;
	wintime = (win_time*)malloc(sizeof(win_time));
	if (wintime == NULL)
	{
		return NULL;
	}

	wintime->time_record = 0;
	return wintime;
}

/** 
 *  \brief		判断是否过期
 *  \note		现在的时间和过去的时间sdktime相比，是否已经超过了time_ms，超过了为过期
 *  \method		Platform_TimeIsExpired_Bydiff
 *  \param[in] 	sdktime 之前的时间对象
 *  \param[in] 	time_ms 时间判断基准
 *  \return 	过期返1 未过期返0
 */
char Platform_TimeIsExpired_Bydiff(ezdev_sdk_time sdktime, EZDEV_SDK_UINT32 time_ms)
{
	win_time* wintime = (win_time*)sdktime;
	time_t now_t = 0;
	if (wintime == NULL)
	{
		return (char)1;
	}

	now_t = time(NULL);
	if ((now_t - wintime->time_record) > (time_ms/1000 + 1))
	{
		return (char)1;
	}
	else
	{
		return (char)0;
	}
}

/** 
 *  \brief		判断是否过期
 *  \note		判断sdktime是否已过期，比现在早的话为过期
 *  \method		Platform_TimerIsExpired
 *  \param[in] 	sdktime 时间对象
 *  \return 	过期返1 未过期返0
 */
char Platform_TimerIsExpired(ezdev_sdk_time sdktime)
{
	time_t now_t = 0;
	win_time* wintime = (win_time*)sdktime;
	if (wintime == NULL)
	{
		return (char)1;
	}

	now_t = time(NULL);

	if (now_t >= wintime->time_record)
	{
		return (char)1;
	}
	else
	{
		return (char)0;
	}
}

/** 
 *  \brief		获取未来多少毫秒后的是时间
 *  \method		Platform_TimerCountdownMS
 *  \param[in] 	sdktime 时间对象
 *  \param[in] 	timeout 未来多少毫秒
 */
void Platform_TimerCountdownMS(ezdev_sdk_time sdktime, unsigned int timeout)
{
	// 	struct timeval now;
	// 	struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
	// 	gettimeofday(&now, NULL);
	// 	timeradd(&now, &interval, &timer->end_time);

	time_t now_t = 0;
	win_time* wintime = (win_time*)sdktime;
	if (wintime == NULL)
	{
		return (char)1;
	}

	now_t = time(NULL);

	wintime->time_record = time(NULL);

	wintime->time_record += (timeout/1000 + 1);
}

/** 
 *  \brief		获取未来多少秒后的是时间
 *  \method		Platform_TimerCountdown
 *  \param[in] 	sdktime 时间对象
 *  \param[in] 	timeout 未来多少秒
 */
void Platform_TimerCountdown(ezdev_sdk_time sdktime, unsigned int timeout)
{
	// 	struct timeval now;
	// 	struct timeval interval = {timeout, 0};
	// 	gettimeofday(&now, NULL);
	// 	timeradd(&now, &interval, &timer->end_time);

	time_t now_t = 0;
	win_time* wintime = (win_time*)sdktime;
	if (wintime == NULL)
	{
		return (char)1;
	}

	now_t = time(NULL);

	wintime->time_record = time(NULL);
	wintime->time_record += timeout;
}

/** 
 *  \brief		获取剩余时间 用来倒计时
 *  \method		Platform_TimerCountdown
 *  \param[in] 	sdktime 倒计时对象
 *  \return 	返回剩余时间 如果已过期返回0
 */
EZDEV_SDK_UINT32 Platform_TimerLeftMS(ezdev_sdk_time sdktime)
{
	// 	struct timeval now, res;
	// 	gettimeofday(&now, NULL);
	// 	timersub(&timer->end_time, &now, &res);
	// 	//printf("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000);
	// 	return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;

	time_t now_t = 0;
	win_time* wintime = (win_time*)sdktime;
	if (wintime == NULL)
	{
		return (char)1;
	}
	now_t = time(NULL);

	if (now_t > wintime->time_record)
	{
		return 0;
	}
	else
	{
		return (wintime->time_record-now_t)* 1000;
	}
}

/** 
 *  \brief		销毁时间对象
 *  \method		Platform_TimeDestroy
 *  \param[in] 	sdktime 时间对象
 */
void Platform_TimeDestroy(ezdev_sdk_time sdktime)
{
	time_t now_t = 0;
	win_time* wintime = (win_time*)sdktime;
	if (wintime == NULL)
	{
		return (char)1;
	}
	
	free(wintime);
	wintime = NULL;
}

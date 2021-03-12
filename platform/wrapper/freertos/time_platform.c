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
#include "esp_common.h"
#include "base_typedef.h"

ezdev_sdk_time Platform_TimerCreater()
{
	freertos_time* freertostime = NULL;
	freertostime = (freertos_time*)malloc(sizeof(freertos_time));
	if (freertostime == NULL)
	{
		return NULL;
	}
	freertostime->end_time = system_get_time();
	return freertostime;
}

char Platform_TimeIsExpired_Bydiff(ezdev_sdk_time sdktime, EZDEV_SDK_UINT32 time_ms)
{
	freertos_time* freertostime = (freertos_time*)sdktime;
	if (freertostime == NULL)
	{
		return (char)1;
	}
	uint32 cur_time = system_get_time();
	if(cur_time - freertostime->end_time >= (time_ms*1000))
	{
		return (char)1;
	}
	else
	{
		return (char)0;
	}
}

char Platform_TimerIsExpired(ezdev_sdk_time sdktime)
{
	freertos_time* freertostime = (freertos_time*)sdktime;
	if (freertostime == NULL)
	{
		return (char)1;
	}
	uint32 cur_time = system_get_time();
	if (cur_time >= freertostime->end_time)
	{
		return (char)1;
	}
	
	return (char)0;
	
}


void Platform_TimerCountdownMS(ezdev_sdk_time sdktime, unsigned int timeout)
{
	freertos_time* freertostime = (freertos_time*)sdktime;
	if (freertostime == NULL)
	{
		return;
	}
	uint32 cur_time = system_get_time();
	freertostime->end_time = cur_time + timeout*1000;
}


void Platform_TimerCountdown(ezdev_sdk_time sdktime, unsigned int timeout)
{
	Platform_TimerCountdownMS(sdktime, timeout*1000);
}


EZDEV_SDK_UINT32 Platform_TimerLeftMS(ezdev_sdk_time sdktime)
{
	freertos_time* freertostime = (freertos_time*)sdktime;
	if (freertostime == NULL)
	{
		return 0;
	}
	uint32 cur_time = system_get_time();
	
	if (freertostime->end_time - cur_time < 0)
	{
		return 0;
	}
	else
	{
		return (freertostime->end_time - cur_time)/1000;
	}
}

void Platform_TimeDestroy(ezdev_sdk_time sdktime)
{
	freertos_time* freertostime = (freertos_time*)sdktime;
	if (freertostime == NULL)
	{
		return;
	}
	
	free(freertostime);
	freertostime = NULL;
}
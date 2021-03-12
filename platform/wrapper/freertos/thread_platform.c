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

#include <unistd.h>
#include "thread_platform_wrapper.h"

static void * sdk_thread_fun(void * aArg)
{
	thread_handle* hd = (thread_handle*)aArg;
	if (hd == NULL)
	{
		return NULL;
	}
	hd->task_do(hd->thread_arg);

	return NULL;
}

ezdev_sdk_mutex sdk_platform_thread_mutex_create()
{
	sdk_mutex_platform* ptr_mutex_platform = NULL;
	ptr_mutex_platform = (sdk_mutex_platform*)malloc(sizeof(sdk_mutex_platform));
	if (ptr_mutex_platform == NULL)
	{
		return NULL;
	}

	ptr_mutex_platform->lock = xSemaphoreCreateMutex();
	if (ptr_mutex_platform->lock == NULL)
	{
		free(ptr_mutex_platform);
		ptr_mutex_platform = NULL;
	}
	return (ezdev_sdk_mutex)ptr_mutex_platform;
}

void sdk_platform_thread_mutex_destroy(ezdev_sdk_mutex ptr_mutex)
{
	sdk_mutex_platform* ptr_mutex_platform = (sdk_mutex_platform*)ptr_mutex;
	if (ptr_mutex_platform == NULL)
	{
		return;
	}

	vSemaphoreDelete(ptr_mutex_platform->lock);

	free(ptr_mutex_platform);
	ptr_mutex_platform = NULL;
	return 0;
}

int sdk_platform_thread_mutex_lock(ezdev_sdk_mutex ptr_mutex)
{
	sdk_mutex_platform* ptr_mutex_platform = (sdk_mutex_platform*)ptr_mutex;
	if (ptr_mutex_platform == NULL)
	{
		return -1;
	}

	if (SemaphoreTake(ptr_mutex_platform->lock, portMAX_DELAY) == pdTRUE)
	{
		return 0;
	}
	return -1;
}

int sdk_platform_thread_mutex_unlock(ezdev_sdk_mutex ptr_mutex)
{
	sdk_mutex_platform* ptr_mutex_platform = (sdk_mutex_platform*)ptr_mutex;
	if (ptr_mutex_platform == NULL)
	{
		return -1;
	}

	if(xSemaphoreGive(ptr_mutex_platform->lock) == pdTRUE)
	{
		return 0;
	}
	return -1;
}

int sdk_thread_create(thread_handle* handle)
{
	if (handle == NULL)
	{
		return -1;
	}
	int rc = 0;
	uint16_t usTaskStackSize = (configMINIMAL_STACK_SIZE * 20);
	unsigned portBASE_TYPE uxTaskPriority = uxTaskPriorityGet(NULL); /* set the priority as the same as the calling task*/

	rc = xTaskCreate(handle->thread_hd,	/* The function that implements the task. */
		handle->thread_name,			/* Just a text name for the task to aid debugging. */
		usTaskStackSize,	/* The stack size is defined in FreeRTOSIPConfig.h. */
		handle->thread_arg,				/* The task parameter, not used in this case. */
		uxTaskPriority,		/* The priority assigned to the task is defined in FreeRTOSConfig.h. */
		&handle->thread_hd);		/* The task handle is not used. */

	if (rc != pdPASS)
	{
		return -1;
	}
	return 0;
}

int sdk_thread_destroy(thread_handle* handle)
{
	if (handle == NULL)
	{
		return -1;
	}
	if(handle->thread_hd != 0)
	{
		vTaskDelete(handle->thread_hd);
	}
	return 0;
}

void sdk_thread_sleep(unsigned int time_ms)
{
	portTickType xTicksToWait = time_ms / portTICK_RATE_MS/*portTICK_PERIOD_MS*/;

	vTaskDelay(xTicksToWait);
}

void sdk_thread_set_name(const char* thread_name)
{
}

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

#include "thread_platform_wrapper.h"
#include <process.h>
#include <windows.h>
#include "ezdev_sdk_kernel_struct.h"

/** 
 *  \brief		线程函数
 *  \method		sdk_thread_fun
 *  \param[in] 	arg	线程上下文
 *  \return 	线程退出码,默认为0
 */
unsigned int __stdcall sdk_thread_fun( void * arg)
{
	thread_handle* hd = (thread_handle*)arg;
	if (hd == NULL)
	{
		return 0;
	}
	hd->task_do(hd->thread_arg);
	return 0;
}

/** 
 *  \brief		线程锁对象创建
 *  \method		sdk_platform_thread_mutex_create
 *  \return 	成功返回线程锁对象 失败返回NULL
 */
ezdev_sdk_mutex sdk_platform_thread_mutex_create()
{
	sdk_mutex_platform* ptr_mutex_platform = NULL;
	ptr_mutex_platform = (sdk_mutex_platform*)malloc(sizeof(sdk_mutex_platform));
	if (ptr_mutex_platform == NULL)
	{
		return NULL;
	}
	InitializeCriticalSection(&ptr_mutex_platform->lock);
	return (ezdev_sdk_mutex)ptr_mutex_platform;
}

/** 
 *  \brief		线程锁对象销毁
 *  \method		sdk_platform_thread_mutex_destroy
 *  \param[in] 	arg	线程锁对象
 */
void sdk_platform_thread_mutex_destroy(ezdev_sdk_mutex ptr_mutex)
{
	sdk_mutex_platform* ptr_mutex_platform = (sdk_mutex_platform*)ptr_mutex;
	if (ptr_mutex_platform == NULL)
	{
		return;
	}
	DeleteCriticalSection(&ptr_mutex_platform->lock);

	free(ptr_mutex_platform);
	ptr_mutex_platform = NULL;
}

/** 
 *  \brief		线程上锁
 *  \method		sdk_platform_thread_mutex_lock
 *  \param[in] 	ptr_mutex 线程锁对象
 *  \return 	成功返回0 失败返回-1
 */
int sdk_platform_thread_mutex_lock(ezdev_sdk_mutex ptr_mutex)
{
	sdk_mutex_platform* ptr_mutex_platform = (sdk_mutex_platform*)ptr_mutex;
	if (ptr_mutex_platform == NULL)
	{
		return -1;
	}
	EnterCriticalSection(&ptr_mutex_platform->lock);
	return 0;
}

/** 
 *  \brief		线程解锁
 *  \method		sdk_platform_thread_mutex_unlock
 *  \param[in] 	ptr_mutex 线程锁对象
 *  \return 	成功返回0 失败返回-1
 */
int sdk_platform_thread_mutex_unlock(ezdev_sdk_mutex ptr_mutex)
{
	sdk_mutex_platform* ptr_mutex_platform = (sdk_mutex_platform*)ptr_mutex;
	if (ptr_mutex_platform == NULL)
	{
		return -1;
	}
	LeaveCriticalSection(&ptr_mutex_platform->lock);
	return 0;
}

int sdk_thread_create(thread_handle* handle)
{
	unsigned int threadID = 0;
	if (handle == NULL)
	{
		return -1;
	}
	handle->thread_hd = (HANDLE)_beginthreadex( NULL, 0, sdk_thread_fun, handle, 0, &threadID);
	if(handle->thread_hd == NULL)
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
	if(handle->thread_hd != NULL)
	{
		WaitForSingleObject(handle->thread_hd, INFINITE);
		CloseHandle(handle->thread_hd);
		handle->thread_hd = NULL;
	}
}

void sdk_thread_sleep(unsigned int time_ms)
{
	Sleep(time_ms);
}
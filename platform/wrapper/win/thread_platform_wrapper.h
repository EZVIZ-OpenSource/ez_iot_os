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

#ifndef H_THREAD_PLATFORM_SWAP_H_
#define H_THREAD_PLATFORM_SWAP_H_

#include <Windows.h>

typedef struct 
{
	CRITICAL_SECTION lock;
}sdk_mutex_platform;

typedef struct thread_handle_platform
{
	HANDLE thread_hd;
	void* thread_arg;
	unsigned int (*task_do)(void * user_data);
	char thread_name[16];
}thread_handle;

#endif
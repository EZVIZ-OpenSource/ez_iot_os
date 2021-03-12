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
#ifndef H_THREAD_INTERFACE_H_
#define H_THREAD_INTERFACE_H_

#include "thread_platform_wrapper.h"

typedef struct  sdk_mutex_platform sdk_mutex;

int sdk_thread_mutex_init(sdk_mutex* mutex);
int sdk_thread_mutex_fini(sdk_mutex* mutex);
int sdk_thread_mutex_lock(sdk_mutex* mutex);
int sdk_thread_mutex_unlock(sdk_mutex* mutex);


typedef struct thread_handle_platform thread_handle;

int sdk_thread_create(thread_handle* handle);
int sdk_thread_destroy(thread_handle* handle);

void sdk_thread_sleep(unsigned int time_ms);

#endif
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


/** 
 *  \brief		创建线程
 *  \method		sdk_thread_create
 *  \param[in] 	handle 线程上下文
 *  \return 	成功返回0 失败返回-1
 */
int sdk_thread_create(thread_handle* handle);

/** 
 *  \brief		线程销毁
 *  \method		sdk_thread_destroy
 *  \param[in] 	handle 线程上下文
 *  \return 	成功返回0 失败返回-1
 */
int sdk_thread_destroy(thread_handle* handle);

/** 
 *  \brief		线程休眠
 *  \method		sdk_thread_sleep
 *  \param[in] 	time_ms 休眠时间（毫秒）
 */
void sdk_thread_sleep(unsigned int time_ms);

//typedef struct thread_handle_platform thread_handle;

#endif
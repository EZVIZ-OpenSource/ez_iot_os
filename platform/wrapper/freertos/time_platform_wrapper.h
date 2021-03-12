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
#ifndef H_TIME_PLATFORM_WRAPPER_H_
#define H_TIME_PLATFORM_WRAPPER_H_

#include "esp_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ezdev_sdk_kernel_struct.h"

typedef struct 
{
	uint32 end_time;
}freertos_time;
#endif
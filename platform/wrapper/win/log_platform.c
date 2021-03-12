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
#include "log_platform_wrapper.h"
#include <stdarg.h>
#include <time.h>

void time_print(int sdk_error, int othercode, const char * buf)
{
	char now_time[16];
	time_t t = time(0);
	strftime(now_time, 16, "%m-%d %H:%M:%S", localtime(&t));

	printf("[%s][%d][%d]sdk_error:%d, othercode:%d, info:%s \n", now_time, getpid(), 0, sdk_error, othercode, buf);
}

void log_print_error(int sdk_error, int othercode, const char * buf)
{
	time_print(sdk_error, othercode, buf);
}

void log_print_warn(int sdk_error, int othercode, const char * buf)
{
	time_print(sdk_error, othercode, buf);
}

void log_print_info(int sdk_error, int othercode, const char * buf)
{
	time_print(sdk_error, othercode, buf);
}

void log_print_debug(int sdk_error, int othercode, const char * buf)
{
	time_print(sdk_error, othercode, buf);
}

void log_print_trace(int sdk_error, int othercode, const char * buf)
{
	time_print(sdk_error, othercode, buf);
}


void log_print(const char *fmt, ...)
{
	va_list ap;
	char logbuf[256];
	memset(logbuf, 0, 256);

	va_start(ap, fmt);
	vsnprintf(logbuf, 256, fmt,ap);
	va_end(ap);

	printf("log_print: %s \n", logbuf);
}
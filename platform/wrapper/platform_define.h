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

#ifndef H_PLATFORM_DEFINE_H_
#define H_PLATFORM_DEFINE_H_

#define LOG_PLATFORM_INTERFACE                                                  \
	extern void log_print_error(int sdk_error, int othercode, const char *buf); \
	extern void log_print_warn(int sdk_error, int othercode, const char *buf);  \
	extern void log_print_info(int sdk_error, int othercode, const char *buf);  \
	extern void log_print_debug(int sdk_error, int othercode, const char *buf); \
	extern void log_print_trace(int sdk_error, int othercode, const char *buf); \
	extern void log_print(const char *fmt, ...);

#define NET_PLATFORM_INTERFACE                                                                                                                                         \
	extern ezdev_sdk_net_work net_create();                                                                                                                            \
	extern ezdev_sdk_kernel_error net_connect(ezdev_sdk_net_work net_work, const char *server_ip, int server_port, int timeout, char szRealIp[ezdev_sdk_ip_max_len]);  \
	extern ezdev_sdk_kernel_error net_read(ezdev_sdk_net_work net_work, unsigned char *read_buf, int read_buf_maxsize, int read_timeout);                              \
	extern ezdev_sdk_kernel_error net_write(ezdev_sdk_net_work net_work, unsigned char *write_buf, int write_buf_size, int send_timeout_ms, int *real_write_buf_size); \
	extern void net_disconnect(ezdev_sdk_net_work net_work);                                                                                                           \
	extern void net_destroy(ezdev_sdk_net_work net_work);                                                                                                              \
	int net_getsocket(ezdev_sdk_net_work net_work);

#define EZDEVSDK_CONFIG_INTERFACE                                                                        \
	extern int get_devinfo_fromconfig(const char *path, char *devinfo_context, int devinfo_context_len); \
	extern int set_file_value(const char *path, unsigned char *keyvalue, int keyvalue_size);             \
	extern int get_file_value(const char *path, unsigned char *keyvalue, int keyvalue_maxsize);

#define TIME_PLATFORM_INTERFACE                                                               \
	extern ezdev_sdk_time Platform_TimerCreater();                                            \
	extern char Platform_TimeIsExpired_Bydiff(ezdev_sdk_time time, EZDEV_SDK_UINT32 time_ms); \
	extern char Platform_TimerIsExpired(ezdev_sdk_time time);                                 \
	extern void Platform_TimerCountdownMS(ezdev_sdk_time time, unsigned int timeout);         \
	extern void Platform_TimerCountdown(ezdev_sdk_time time, unsigned int timeout);           \
	extern EZDEV_SDK_UINT32 Platform_TimerLeftMS(ezdev_sdk_time time);                        \
	extern void Platform_TimeDestroy(ezdev_sdk_time time);                                    \
	extern void sdk_thread_sleep(unsigned int time_ms);

#define MUTEX_PLATFORM_INTERFACE                                              \
	extern ezdev_sdk_mutex sdk_platform_thread_mutex_create();                \
	extern void sdk_platform_thread_mutex_destroy(ezdev_sdk_mutex ptr_mutex); \
	extern int sdk_platform_thread_mutex_lock(ezdev_sdk_mutex ptr_mutex);     \
	extern int sdk_platform_thread_mutex_unlock(ezdev_sdk_mutex ptr_mutex);
#endif //H_PLATFORM_DEFINE_H_
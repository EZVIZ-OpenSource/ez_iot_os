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

#ifndef H_EZDEVSDK_CONFIG_H_
#define H_EZDEVSDK_CONFIG_H_

/** 
 *  \brief		devinfo配置文件的读取
 *  \method		get_devinfo_fromconfig
 *  \param[in]	path				路径
 *  \param[out]	devinfo_context		数据地址
 *  \param[in]	devinfo_context_len	数据长度
 *  \returns    成功返回0,失败返回-1
 */
int get_devinfo_fromconfig(const char* path, char* devinfo_context, int devinfo_context_len);

/** 
 *  \brief		数据保存至文件，目前用于devid和masterkey
 *  \method		set_file_value
 *  \param[in]	path				路径
 *  \param[in]	devinfo_context		数据地址
 *  \param[in]	devinfo_context_len	数据长度
 *  \returns    成功返回0,失败返回-1
 */
int set_file_value(const char* path, unsigned char* keyvalue, int keyvalue_size);

/** 
 *  \brief		从文件读取，目前用于devid和masterkey，读取长度小于64字节
 *  \method		get_file_value
 *  \param[in]	path				路径
 *  \param[out]	devinfo_context		数据地址
 *  \param[in]	devinfo_context_len	数据长度
 *  \returns    成功返回0,失败返回-1
 */
int get_file_value(const char* path, unsigned char* keyvalue, int keyvalue_maxsize);

#endif
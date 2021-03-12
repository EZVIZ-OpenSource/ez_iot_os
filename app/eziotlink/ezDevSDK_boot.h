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

#ifndef H_EZDEVSDK_BOOT_H_ 
#define H_EZDEVSDK_BOOT_H_

#include "base_typedef.h"

#if defined (_WIN32) || defined(_WIN64)

#ifdef EZDEVSDK_BOOT_EXPORTS
#define EZDEVSDK_BOOT_API	__declspec(dllexport)
#else
#define EZDEVSDK_BOOT_API __declspec(dllimport)
#endif

#define EZDEVSDK_BOOT_CALLBACK __stdcall
#else
#define EZDEVSDK_BOOT_API 

#define EZDEVSDK_BOOT_CALLBACK 
#endif

#define ezdev_sdk_ip_max_len			64		///<	ip最长长度
#define ezdev_sdk_sessionkey_len		16		///<	设备会话秘钥长度 ,默认是16个字节
#define ezdev_sdk_name_len				64		///<	设备SDK 一些命名的长度

/*! \addtogroup Boot_Module SDKBoot模块
 * SDKBoot模块，向上层应用提供初始化、反初始化、启动、停止微内核等功能接口，承接时间、线程、网络等和平台关联性大的功能实现
 *  \{
 */


/**
 * \brief App通知消息
 */
typedef enum
{
	ezDevSDK_App_Event_Online,				          ///<	设备上线	context == sdk_sessionkey_context
	ezDevSDK_App_Event_Break,				          ///<	设备下线	context == sdk_offline_context
	ezDevSDK_App_Event_Switchover,			          ///<	平台切换	context == sdk_switchover_context
	ezDevSDK_App_Event_Invaild_authcode,	          ///<	验证码不合规且没有绑定用户，交给上层APP处理	event_context == null
	ezDevSDK_App_Event_fast_reg_online,		          ///<	event_context == sdk_sessionkey_context	设备快速上线
	ezDevSDK_App_Event_Runtime_err,			          ///<	evnet_context == sdk_runtime_err_context 设备sdk运行时错误信息
	ezDevSDK_App_Event_Reconnect_success,             ///<  evnet_context == NULL 重连成功事件回调
	ezDevSDK_App_Event_heartbeat_interval_changed     ///<  evnet_context == int  心跳改变事件回调
}ezDevSDK_App_Event;

/**
 * \brief 日志级别
 */
typedef enum
{
	ezDevSDK_log_error,				///<	错误
	ezDevSDK_log_warn,				///<	警告
	ezDevSDK_log_info,				///<	信息
	ezDevSDK_log_debug,				///<	调试
	ezDevSDK_log_trace				///<	轨迹
}ezDevSDK_App_LogLevel;

/**
 * \brief 存取数据类型
 */
typedef enum
{
	ezDevSDK_keyvalue_devid,				///<	设备唯一标识  首次设备上线后会分配 一定要写入flash
	ezDevSDK_keyvalue_masterkey,			///<	设备masterkey 首次设备上线后会分配 尽量写入flash
	ezDevSDK_keyvalue_count					///<	此枚举上限
}ezDevSDK_App_keyvalue_type;

/**
 * \brief 存取数据类型
 */
typedef enum
{
	ezDevSDK_curingdata_secretkey,			///<	验证码不合规设备重新申请的secretkey，一定要固化
	ezDevSDK_curingdata_count				///<	此枚举上限
}ezDevSDK_App_curingdata_type;

/** 
 *  \brief		定义事件通知函数
 *  \method		ezDevSDK_Event_Notice
 *  \param[in] 	event_id				消息类型
 *  \param[in] 	context					消息相关联的数据
 */
typedef void (*ezDevSDK_Event_Notice)(ezDevSDK_App_Event event_id, void * context);

/** 
 *  \brief		定义日志通知函数
 *  \method		ezDevSDK_key_value_load
 *  \param[in] 	level					日志等级
 *  \param[in] 	sdk_error				SDK错误码
 *  \param[in] 	othercode				其他的错误码
 *  \param[in] 	buf						消息存放地址
 */
typedef void (*ezDevSDK_Log_Notice)(ezDevSDK_App_LogLevel level, EZDEV_SDK_INT32 sdk_error, EZDEV_SDK_INT32 othercode, const char *buf);

/** 
 *  \brief		定义加载关键信息的函数
 *  \method		ezDevSDK_key_value_load
 *  \param[in] 	valuetype				数据类型
 *  \param[in] 	keyvalue				数据存放地址
 *  \param[in] 	keyvalue_maxsize		数据存放最大值
 */
typedef void (*ezDevSDK_key_value_load)(ezDevSDK_App_keyvalue_type valuetype, unsigned char* keyvalue, EZDEV_SDK_UINT32 keyvalue_maxsize);

/** 
 *  \brief		定义存储关键信息的函数
 *  \method		ezDevSDK_key_value_save
 *  \param[in] 	valuetype				数据类型
 *  \param[in] 	keyvalue				传入数据地址
 *  \param[in] 	keyvalue_size			传入数据长度
 *  \return 	成功返回0 失败返回非0
 */
typedef EZDEV_SDK_INT32 (*ezDevSDK_key_value_save)(ezDevSDK_App_keyvalue_type valuetype, unsigned char* keyvalue, EZDEV_SDK_UINT32 keyvalue_size);

/** 
 *  \brief			定义加载关键信息的函数
 *  \method			ezDevSDK_curing_data_load
 *  \param[in] 		valuetype				数据类型
 *  \param[in] 		keyvalue				数据存放地址
 *  \param[in\out] 	*keyvalue_maxsize		数据缓冲区最大值,返回数据大小
 *  \return 		成功返回0，失败返回非0
 */
typedef EZDEV_SDK_INT32 (*ezDevSDK_curing_data_load)(ezDevSDK_App_curingdata_type valuetype, unsigned char* keyvalue, EZDEV_SDK_UINT32 *keyvalue_maxsize);

/** 
 *  \brief		定义存储关键信息的函数
 *  \method		ezDevSDK_curing_data_save
 *  \param[in]	valuetype				数据类型
 *  \param[in]	keyvalue				传入数据地址
 *  \param[in] 	keyvalue_size			传入数据长度
 *  \return 	成功返回0，失败返回非0
 */
typedef EZDEV_SDK_INT32 (*ezDevSDK_curing_data_save)(ezDevSDK_App_curingdata_type valuetype, unsigned char* keyvalue, EZDEV_SDK_UINT32 keyvalue_size);

/**
 * 低功耗设备快速上线.
 */
typedef struct
{
	EZDEV_SDK_INT8 bLightreg;							///< 指定是否快速上线,0(否),1是wifi快速重连，2是RF快速重连
	EZDEV_SDK_UINT16 das_port;							///< das端口
	EZDEV_SDK_UINT16 das_udp_port;						///< das udp端口
	int das_socket;										///< 上次下线保修的socket,可以不指定
	char das_address[ezdev_sdk_ip_max_len];				///< das IP地址
	char das_domain[ezdev_sdk_ip_max_len];				///< das 域名
	char das_serverid[ezdev_sdk_name_len];				///< das serverid
	unsigned char session_key[ezdev_sdk_sessionkey_len];///< das session key
}ezDevSDK_das_info;

/**
 * 初始化微内核的结构体.
 * \note
 * - 如果bUser为!0，需要实现存取回调函数并在初始化时候传入
 * - 启动微内核前devinfo的文件应存在且有效
 * - 启动微内核前dev_id和dev_masterkey可以只提供路径，在设备上线后得到数据会根据此路径写文件
 */
typedef struct
{
	EZDEV_SDK_UINT8 bUser;								///< 是否开启用户回调 0关闭 !0开启 如果为0 dev_id&masterkey通过文件方式存取 否则通过回调的方式存取
	ezDevSDK_key_value_load	keyValueLoadFun;			///< 读信息的函数
	ezDevSDK_key_value_save keyValueSaveFun;			///< 写信息的函数
	ezDevSDK_curing_data_load	curingDataLoadFun;		///< 固化数据的读函数,必须实现
	ezDevSDK_curing_data_save 	curingDataSaveFun;		///< 固化数据的写函数,必须实现
	char devinfo_path[128];								///< devinfo文件路径
	char dev_id[128];									///< dev_id文件路径
	char dev_masterkey[128];							///< masterkey文件路径
	ezDevSDK_das_info* reg_das_info;					///< 低功耗设备快速上线,需要提供das信息,如果不需要默认为NULL
}ezDevSDK_config;

typedef struct
{
	ezDevSDK_Event_Notice event_notice;			///<	事件通知回调
	ezDevSDK_Log_Notice log_notice;				///<	日志回调
}ezDevSDK_notice;

typedef struct
{
	ezDevSDK_notice	notice;						///<	消息通知相关
	ezDevSDK_config config;						///<	配置相关
}ezDevSDK_all_config;

#ifdef __cplusplus
extern "C"
{
#endif

/** 
 *  \brief		Boot模块初始化接口
 *  \method		ezDevSDK_Init
 *  \param[in] 	server_name			平台服务域名
 *  \param[in] 	server_port			平台服务端口
 *  \param[in] 	all_config			配置信息
 *  \param[in]  reg_mode			注册模式
 *  \details
 *	1---正常设备(平台默认值)
 *	2---wifi托管低功耗设备(现已有,表示电池设备当前托管状态)
 *	3---RF托管低功耗设备(本次新增, 表示电池设备当前托管状态)
 *	4---RF管理(本次新增, 表示支持RF托管,由Base设备上报)
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_params_invalid、ezdev_sdk_kernel_json_invalid、ezdev_sdk_kernel_json_format、 \n
 *				ezdev_sdk_kernel_value_load、ezdev_sdk_kernel_invald_call、ezdev_sdk_kernel_memory、ezdev_sdk_kernel_internal
 *	\see		错误码 ： ezdev_sdk_kernel_error
 */
EZDEVSDK_BOOT_API  EZDEV_SDK_INT32 ezDevSDK_Init(const char* server_name, EZDEV_SDK_UINT32 server_port, ezDevSDK_all_config* all_config, EZDEV_SDK_UINT32 reg_mode);

/** 
 *  \brief		Boot模块反初始化接口
 *  \method		ezDevSDK_Fini
 *  \return 	成功返回0 失败详见错误码
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call
 *	\see		错误码 ： ezdev_sdk_kernel_error
 */
EZDEVSDK_BOOT_API  EZDEV_SDK_INT32 ezDevSDK_Fini();

/** 
 *  \brief		Boot模块启动接口（开启整个SDK，调用完接口后，SDK进入工作模式）
 *  \method		ezDevSDK_Start
 *  \return 	成功返回0 失败详见错误码
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call
 *	\see		错误码 ： ezdev_sdk_kernel_error
 */
EZDEVSDK_BOOT_API  EZDEV_SDK_INT32 ezDevSDK_Start();

/** 
 *  \brief		Boot模块停止接口
 *  \method		ezDevSDK_Stop
 *  \return 	成功返回0 失败详见错误码
 *  \return 	ezdev_sdk_kernel_succ、ezdev_sdk_kernel_invald_call
 *	\see		错误码 ： ezdev_sdk_kernel_error
 */
EZDEVSDK_BOOT_API  EZDEV_SDK_INT32 ezDevSDK_Stop();

#ifdef __cplusplus
}
#endif
/*! \} */

#endif //H_EZDEVSDK_BOOT_H_
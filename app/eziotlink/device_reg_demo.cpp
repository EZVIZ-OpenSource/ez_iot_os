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

#if defined(_WIN32) || defined(_WIN64)
#include <process.h>
#define  gettid() GetCurrentProcessId()

#pragma comment(lib, "../lib/ezDevSDK_boot.lib")

#else
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#define  gettid() syscall(SYS_gettid)
#endif
#include <time.h>
#include "inireader.h"
#include "base_typedef.h"
#include "ezDevSDK_boot.h"
#include "ezdev_sdk_kernel_error.h"
#include "ezdev_sdk_kernel_struct.h"
#include "ezdev_sdk_kernel.h"

#define CONFIG_LOACL_DIR	"./fileDepens/"

sdk_sessionkey_context	g_skey_ctx = {0};

void virtual_domain_start_cb(EZDEV_SDK_PTR pUser)
{
	//TODO
	/* 开发者自己处理,如果不需要可以空实现 */
}

void virtual_domain_stop_cb(EZDEV_SDK_PTR pUser)
{
	//TODO
	/* 开发者自己处理,如果不需要可以空实现 */
}

void das_data_route_cb(ezdev_sdk_kernel_submsg_v3* ptr_submsg)
{
	int ret = 0;
	int code = 0;
	if(NULL == ptr_submsg->buf)
	{
		printf("recieve das msg is null\n");
		return ;
	}
}

void event_router_cb(ezdev_sdk_kernel_event* ptr_event)
{
	//TODO	
	switch(ptr_event->event_type)
	{
	case ezDevSDK_App_Event_Runtime_err:
		{
			printf("-------------------------APP get event ezDevSDK_App_Event_Runtime_err_v3----------------------------\n");
			sdk_runtime_err_context* rt_err_ctx = (sdk_runtime_err_context*)ptr_event->event_context;
			printf("tag:%d, errcode:0x%08x\n", rt_err_ctx->err_tag, rt_err_ctx->err_code);
			if(TAG_MSG_ACK_V3== rt_err_ctx->err_tag)
			{
				//信令发送回执
				sdk_send_msg_ack_context_v3* ptr_msg_ack = (sdk_send_msg_ack_context_v3*)rt_err_ctx->err_ctx;
				printf("module:%s, method:%s,res_id:%s,res_type:%s, msg_type:%s, ext_msg:%s, msg_seq:%d, msg_qos:%d\n", ptr_msg_ack->module, ptr_msg_ack->method, 
						ptr_msg_ack->resource_id, ptr_msg_ack->resource_type, ptr_msg_ack->msg_type,ptr_msg_ack->ext_msg,ptr_msg_ack->msg_seq,ptr_msg_ack->msg_qos);
			}
			break;
		}
	default:
		{
			break;
		}
	}
}

/** 
 * @brief 注册一个模块
 * @return void
 */
ezdev_sdk_kernel_error domainImp(const char* module)
{
	ezdev_sdk_kernel_extend_v3 extern_info = {0};
	ezdev_sdk_kernel_error ezRv = ezdev_sdk_kernel_succ;

	extern_info.ezdev_sdk_kernel_data_route = das_data_route_cb;
	extern_info.ezdev_sdk_kernel_event_route = event_router_cb;
	strncpy(extern_info.module, module, sizeof(extern_info.module)-1);

	ezRv = ezdev_sdk_kernel_extend_load_v3(&extern_info);
	return ezRv;
}

void event_notice_callback(ezDevSDK_App_Event event_id, void * context)
{	
	//TODO
	/* 开发者添加自己的业务逻辑 */
	int i=0;
	switch(event_id)
	{
	case ezDevSDK_App_Event_Online:
		{
			printf("-------------------------APP get event ezDevSDK_App_Event_Online----------------------------\n");
			memcpy(&g_skey_ctx, context, sizeof(g_skey_ctx));
			break;
		}
	case ezDevSDK_App_Event_Break:
		{
			printf("-------------------------APP get event ezDevSDK_App_Event_Break----------------------------\n");
			break;
		}
	case ezDevSDK_App_Event_Switchover:
		{
			printf("-------------------------APP get event ezDevSDK_App_Event_Switchover----------------------------\n");
			break;
		}
	case ezDevSDK_App_Event_Invaild_authcode:
		{
			printf("-------------------------APP get event ezDevSDK_App_Event_Invaild_authcode----------------------------\n");
			break;
		}
	case ezDevSDK_App_Event_fast_reg_online:
		{
			printf("-------------------------APP get event ezDevSDK_App_Event_fast_reg_online----------------------------\n");
			break;
		}
	case ezDevSDK_App_Event_Runtime_err:
		{
			printf("-------------------------APP get event ezDevSDK_App_Event_Runtime_err----------------------------\n");
			sdk_runtime_err_context* rt_err_ctx = (sdk_runtime_err_context*)context;
			printf("tag:%d, errcode:0x%08x\n", rt_err_ctx->err_tag, rt_err_ctx->err_code);
			if(TAG_MSG_ACK_V3== rt_err_ctx->err_tag)
			{
				//信令发送回执
				sdk_send_msg_ack_context_v3* ptr_msg_ack = (sdk_send_msg_ack_context_v3*)rt_err_ctx->err_ctx;
				printf("module:%s, method:%s,res_id:%s,res_type:%s, msg_type:%s, ext_msg:%s, msg_seq:%d, msg_qos:%d\n", ptr_msg_ack->module, ptr_msg_ack->method, 
						ptr_msg_ack->resource_id, ptr_msg_ack->resource_type, ptr_msg_ack->msg_type,ptr_msg_ack->ext_msg,ptr_msg_ack->msg_seq,ptr_msg_ack->msg_qos);
			}
			break;
		}
	case ezDevSDK_App_Event_Reconnect_success:
	    {
		    printf("-------------------------APP get event ezDevSDK_App_Event_Reconnect_success----------------------------\n");
			break;
	    }
	case ezDevSDK_App_Event_heartbeat_interval_changed:
	    {
		    printf("-------------------------APP get event ezDevSDK_App_Event_keepalive_interval_changed----------------------------\n");
			break;
		}
	default:
		{
			break;
		}
	}
}

void log_notice_callback(ezDevSDK_App_LogLevel level, int sdk_error, int othercode, const char *buf)
{	
	char now_time[16];
	time_t t = time(0);
	strftime(now_time, 16, "%m-%d %H:%M:%S", localtime(&t));

	printf("[%d][%s] sdk_error:%d, othercode:%d, info:%s \n",level ,now_time, sdk_error, othercode, buf);
	//TODO
	/* 开发者自己处理日志 */
}

void key_value_load_callback(ezDevSDK_App_keyvalue_type valuetype, unsigned char* keyvalue, EZDEV_SDK_UINT32 keyvalue_maxsize)
{
	switch(valuetype)
	{
	case ezDevSDK_keyvalue_devid:
		{
			//TODO
			/* 开发者需要实现devid的读取操作 */
		}
		break;
	case  ezDevSDK_keyvalue_masterkey:
		{
			//TODO
			/* 开发者需要实现masterkey的读取操作 */
		}
		break;
	default:
		{
			if (ezDevSDK_keyvalue_count <= valuetype)
			{
				printf("Invalid valuetype\n");
			}
		}
	}
}
EZDEV_SDK_INT32 key_value_save_callback(ezDevSDK_App_keyvalue_type valuetype, unsigned char* keyvalue, EZDEV_SDK_UINT32 keyvalue_size)
{
	EZDEV_SDK_INT32 ezRv = ezdev_sdk_kernel_succ;

	switch(valuetype)
	{
	case ezDevSDK_keyvalue_devid:
		{
			//TODO
			/* 开发者需要实现devid的写入操作,永久固化到flash,确保断电或者设备重置,这个值都不能丢失 */
		}
		break;
	case  ezDevSDK_keyvalue_masterkey:
		{
			//TODO
			/* 开发者需要实现masterkey的写入操作 ,需要固化到flash,有效期一般为三个月,过期自动更新*/
		}
		break;
	default:
		{
			if (ezDevSDK_keyvalue_count <= valuetype)
			{
				printf("Invalid valuetype\n");
				ezRv = ezdev_sdk_kernel_params_invalid;
			}
		}
	}

	return ezRv;
}

EZDEV_SDK_INT32 curing_data_load_cb(ezDevSDK_App_curingdata_type datatype, unsigned char* data, EZDEV_SDK_UINT32 *data_maxsize)
{
	EZDEV_SDK_INT32 ezRv = ezdev_sdk_kernel_succ;

	if(datatype >= ezDevSDK_curingdata_count)
	{
		ezRv = ezdev_sdk_kernel_params_invalid;
		return ezRv;
	}

	if(ezDevSDK_curingdata_secretkey == datatype)
	{
		//TODO
		/* 验证码不合规设备需要处理返回固化的secretkey。如果没有secretkey,返回长度0,用devinfo中的验证码用于登录 */
		*data_maxsize = 0;
	}
	
	return ezRv;
}

EZDEV_SDK_INT32 curing_data_save_cb(ezDevSDK_App_curingdata_type datatype, unsigned char* data, EZDEV_SDK_UINT32 data_size)
{
	EZDEV_SDK_INT32 ezRv = ezdev_sdk_kernel_succ;

	if(datatype >= ezDevSDK_curingdata_count)
	{
		ezRv = ezdev_sdk_kernel_params_invalid;
		return ezRv;
	}

	if(ezDevSDK_curingdata_secretkey == datatype)
	{
		//TODO
		/* 验证码不合规设备需要处理secretkey的固化 */
	}
	
	return ezRv;
}

int main(void)
{
	signal(SIGPIPE,SIG_IGN);
	std::string lbs_domain;
    int lbs_port = 0;
	int ezKernelRv = ezdev_sdk_kernel_succ;
	ezDevSDK_all_config allconfig = {0};

	ezviz_test_ezdevice::CIniReader iniReader;
    iniReader.LoadKeyMap("./ezdevice_config.ini");

    iniReader.GetString("lbs", "domain", lbs_domain);
    iniReader.GetInteger("lbs", "port", lbs_port);

	sprintf(allconfig.config.devinfo_path, "%s%s", CONFIG_LOACL_DIR, "dev_info");
	/**
	 *	用户自由选择dev_id和dev_masterkey通过文件或者回调函数进行存取
	 */
#if 1
	allconfig.config.bUser = 0;															///<	文件的方式存取 bUser为0
	sprintf(allconfig.config.dev_id, "%s%s", CONFIG_LOACL_DIR, "dev_id");
	sprintf(allconfig.config.dev_masterkey, "%s%s", CONFIG_LOACL_DIR, "dev_masterkey");
#else
	allconfig.config.bUser = !0;														///<	回调的方式bUser为1
	allconfig.config.keyValueLoadFun = key_value_load_callback;							///<	回调的方式须实现读函数
	allconfig.config.keyValueSaveFun = key_value_save_callback;							///<	回调的方式须实现写函数
#endif

	allconfig.config.curingDataLoadFun = curing_data_load_cb;							///<	回调的方式须实现读函数
	allconfig.config.curingDataSaveFun = curing_data_save_cb;							///<	回调的方式须实现写函数

	allconfig.notice.event_notice = event_notice_callback;
	allconfig.notice.log_notice = log_notice_callback;

	while (1)
    {
        char message[100];
        fputs("init(fast reg) :0\n", stdout);
		fputs("init           :1\n", stdout);
        fputs("start          :2\n", stdout);
        fputs("stop           :3\n", stdout);
        fputs("final          :4\n", stdout);
        fputs("binding nic    :5\n", stdout);
		fputs("send msg       :6\n", stdout);
		fputs("show svr info  :7\n", stdout);
		fputs("show ver       :8\n", stdout);
        fputs("show key info  :9\n", stdout);
		fputs("RF(fast reg)   :10\n", stdout);
		fputs("exit           :q\n", stdout);

        fgets(message, 100, stdin);

        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n")) 
        {
            break;
        }
		if(!strcmp(message, "0\n"))
		{
			ezDevSDK_das_info pDas_info = {0};
			pDas_info.bLightreg 	= 1;
			pDas_info.das_port 		= g_skey_ctx.das_port;
			pDas_info.das_udp_port 	= g_skey_ctx.das_udp_port;
			memcpy(pDas_info.das_address, g_skey_ctx.das_ip, ezdev_sdk_ip_max_len);
			memcpy(pDas_info.das_domain, g_skey_ctx.das_domain, ezdev_sdk_ip_max_len);
			memcpy(pDas_info.das_serverid, g_skey_ctx.das_serverid, ezdev_sdk_name_len);
			memcpy(pDas_info.session_key, g_skey_ctx.session_key, ezdev_sdk_sessionkey_len);
			allconfig.config.reg_das_info = &pDas_info;

			ezKernelRv = ezDevSDK_Init(lbs_domain.c_str(), lbs_port, &allconfig, 2);
			if (ezdev_sdk_kernel_succ == ezKernelRv)
				printf("-------------Succ-------------\n");
			else
				printf("An error occur in ezDevSDK_Init(fast), error code:0x%08X\n", ezKernelRv);
		}
        if (!strcmp(message, "1\n"))
        {
			ezKernelRv = ezDevSDK_Init(lbs_domain.c_str(), lbs_port, &allconfig, 1);
			if (ezdev_sdk_kernel_succ == ezKernelRv)
				printf("-------------Succ-------------\n");
			else
				printf("An error occur in ezDevSDK_Init, error code:0x%08X\n", ezKernelRv);

			ezKernelRv = domainImp("model");
			if(ezdev_sdk_kernel_succ == ezKernelRv)
			{
                printf("-------------domainImp model Succ-------------\n");
			}else{
				printf("-------------domainImp model error-------------\n");
			}
		}
		if (!strcmp(message, "50\n"))
        {
			ezKernelRv = ezDevSDK_Init(lbs_domain.c_str(), lbs_port, &allconfig, 1);
			if (ezdev_sdk_kernel_succ == ezKernelRv)
				printf("-------------Succ-------------\n");
			else
				printf("An error occur in ezDevSDK_Init, error code:0x%08X\n", ezKernelRv);

		}
        if (!strcmp(message, "2\n"))
        {
			ezKernelRv = ezDevSDK_Start();
			if (ezdev_sdk_kernel_succ == ezKernelRv)
				printf("-------------Succ-------------\n");
			else
				printf("An error occur in ezDevSDK_Start, error code:0x%08X\n", ezKernelRv);
        }
        if (!strcmp(message, "3\n"))
        {
			ezKernelRv = ezDevSDK_Stop();
			if (ezdev_sdk_kernel_succ == ezKernelRv)
				printf("-------------Succ-------------\n");
			else
				printf("An error occur in ezDevSDK_Stop, error code:0x%08X\n", ezKernelRv);
        }
        if (!strcmp(message, "4\n"))
        {
			ezKernelRv = ezDevSDK_Fini();
			if (ezdev_sdk_kernel_succ == ezKernelRv)
				printf("-------------Succ-------------\n");
			else
				printf("An error occur in ezDevSDK_Fini, error code:0x%08X\n", ezKernelRv);
        }
        if(!strcmp(message, "5\n"))
        {
			stun_info hstun_info = {0};
			ezKernelRv = ezdev_sdk_kernel_set_net_option(1, "lo", strlen("lo"));
			if (ezdev_sdk_kernel_succ == ezKernelRv)
				printf("-------------Succ-------------\n");
			else
				printf("An error occur in ezdev_sdk_kernel_set_net_option, error code:0x%08X\n", ezKernelRv);
        }
		if(!strcmp(message, "7\n"))
		{
			server_info_s server_info[10] = {0};
			int count = sizeof(server_info)/sizeof(server_info_s);
			ezKernelRv =  ezdev_sdk_kernel_get_server_info(server_info, &count);
			if (ezdev_sdk_kernel_succ == ezKernelRv)
			{
				printf("-------------Succ-------------\n");
				printf("count = %d\n", count);
			}
			else
			{
				printf("An error occur in ezdev_sdk_kernel_get_server_info, error code:0x%08X\n", ezKernelRv);
			}
		}
		if(!strcmp(message, "8\n"))
		{
			char buf[100] = {0};
			int buflen = sizeof(buf);
			ezKernelRv =  ezdev_sdk_kernel_get_sdk_version(buf, &buflen);
			if (ezdev_sdk_kernel_succ == ezKernelRv)
			{
				printf("-------------Succ-------------\n");
				printf("buf =%s, buflen = %d\n", buf, buflen);
			}
			else
			{
				printf("An error occur in ezdev_sdk_kernel_get_sdk_version, error code:0x%08X\n", ezKernelRv);
			}
		}
		if(!strcmp(message, "9\n"))
		{
			showkey_info keyinfo ={0};
			ezKernelRv =  ezdev_sdk_kernel_show_key_info(&keyinfo);
			if (ezdev_sdk_kernel_succ == ezKernelRv)
			{
				printf("-------------Succ-------------\n");
				printf("masterkey = %s, dev_id = %s,dev_verification_code = %s\n", keyinfo.master_key, keyinfo.dev_id,keyinfo.dev_verification_code);
			}
			else
			{
				printf("An error occur in ezdev_sdk_kernel_show_key_info, error code:0x%08X\n", ezKernelRv);
			}
		}

		if(!strcmp(message, "10\n"))
		{
			ezDevSDK_das_info pDas_info = {0};
			pDas_info.bLightreg 	= 2;
			pDas_info.das_port 		= g_skey_ctx.das_port;
			pDas_info.das_udp_port 	= g_skey_ctx.das_udp_port;
			memcpy(pDas_info.das_address, g_skey_ctx.das_ip, ezdev_sdk_ip_max_len);
			memcpy(pDas_info.das_domain, g_skey_ctx.das_domain, ezdev_sdk_ip_max_len);
			memcpy(pDas_info.das_serverid, g_skey_ctx.das_serverid, ezdev_sdk_name_len);
			memcpy(pDas_info.session_key, g_skey_ctx.session_key, ezdev_sdk_sessionkey_len);
			allconfig.config.reg_das_info = &pDas_info;

			ezKernelRv = ezDevSDK_Init(lbs_domain.c_str(), lbs_port, &allconfig, 3);
			if (ezdev_sdk_kernel_succ == ezKernelRv)
				printf("-------------RF Fast reg Succ-------------\n");
			else
				printf("An error occur in ezDevSDK_Init(RF fast reg), error code:0x%08X\n", ezKernelRv);
		}
	}
	
	return 0;
}

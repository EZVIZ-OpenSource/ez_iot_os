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

#include "FreeRTOS.h"
#include "task.h"
#include "diag.h"
#include "main.h"
#include "platform_define.h"
#include "ezdev_sdk_kernel.h"
#include "semphr.h"

#include <platform_opts.h>
#include <platform/platform_stdlib.h>
#include <lwip_netconf.h>
#include <lwip/sockets.h>
#include "wifi_constants.h"
#include "wifi_structures.h"
#include "lwip_netconf.h"
#include "ezxml.h"
#include "bscJSON.h"
#define DEVINFO_STRING	"{	\
							\"dev_status\":1,	\
							\"dev_subserial\":\"999999998\",	\
							\"dev_verification_code\":\"FEDCBA\",	\
							\"dev_serial\":\"CS-C4S-52WEFR01120140310CCRR999999998\",	\
							\"dev_firmwareversion\":\"V5.1.0 build 150401\",	\
							\"dev_type\":\"CS-C4S\",	\
							\"dev_typedisplay\":\"CS-C4S-52WEFR\",	\
							\"dev_mac\":\"0\",	\
							\"dev_nickname\":\"test\",	\
							\"dev_firmwareidentificationcode\":\"0\"	\
						}"

EZDEVSDK_CONFIG_INTERFACE
NET_PLATFORM_INTERFACE
LOG_PLATFORM_INTERFACE
TIME_PLATFORM_INTERFACE
MUTEX_PLATFORM_INTERFACE
void event_notice_from_sdk_kernel(sdk_kernel_event_type event_type, void* event_context)
{
 
}

void sdk_kernel_logprint(sdk_log_level level, EZDEV_SDK_INT32 sdk_error, EZDEV_SDK_INT32 othercode, const char * buf)
{
	switch(level)
	{
	case sdk_log_error:
		log_print_error(sdk_error, othercode, buf);
		break;
	case sdk_log_warn:
		log_print_warn(sdk_error, othercode, buf);
		break;
	case sdk_log_info:
		log_print_info(sdk_error, othercode, buf);
		break;
	case sdk_log_debug:
		log_print_debug(sdk_error, othercode, buf);
		break;
	case sdk_log_trace:
		log_print_trace(sdk_error, othercode, buf);
		break;
	default:
		break;
	}
}

void value_load(sdk_keyvalue_type valuetype, unsigned char* keyvalue, EZDEV_SDK_INT32 keyvalue_maxsize)
{
	return;
}   

EZDEV_SDK_INT32 value_save(sdk_keyvalue_type valuetype, unsigned char* keyvalue, EZDEV_SDK_INT32 keyvalue_size)
{
	return -1;
}

int send_msg_to_platform(unsigned char* buf, int buflen, int domain_id,int cmd_id, const char* cmd_version)
{
	ezdev_sdk_kernel_pubmsg pubmsg;
	memset(&pubmsg, 0, sizeof(ezdev_sdk_kernel_pubmsg));

	pubmsg.msg_response = 0;
	pubmsg.msg_seq = 0;

	pubmsg.msg_body = buf;
	pubmsg.msg_body_len = buflen;

	pubmsg.msg_domain_id = domain_id;
	pubmsg.msg_command_id = cmd_id;

	strncpy(pubmsg.command_ver, cmd_version, version_max_len);

	ezdev_sdk_kernel_error sdk_error  = ezdev_sdk_kernel_send(&pubmsg);
	if (sdk_error != ezdev_sdk_kernel_succ)
	{
		return -1;
	}
	return 0;
}

void extend_basefuntion_start_cb(EZDEV_SDK_PTR pUser)
{
}

void extend_basefuntion_stop_cb(EZDEV_SDK_PTR pUser)
{
}

void extend_basefuntion_data_route_cb(ezdev_sdk_kernel_submsg* ptr_submsg ,EZDEV_SDK_PTR pUser)
{

     	/**
	 * \brief   constructor of CPerson
	 */
        ezxml_t RootData=NULL;
	int dwCMD = 0;
	if (ptr_submsg == NULL)
	{
		return;
	}
        DBG_8195A("extend_data_route_cb  %s\r\n", ( char *)ptr_submsg->buf);
        RootData = ezxml_parse_str(( char *)ptr_submsg->buf, ptr_submsg->buf_len);
        if (0 != strlen(ezxml_error(RootData)))
        {
            DBG_8195A("ezxml_parse_str error2 %s\r\n", ezxml_error(RootData));
            return;
        }
	dwCMD = ptr_submsg->msg_command_id;
        DBG_8195A("extend_basefuntion_data_route_cb  %d %d\r\n", ptr_submsg->msg_command_id,ptr_submsg->msg_seq);
	int ret = 0;
	switch(dwCMD)
	{
        case 0x300f: /* 开关控制 -- 老接口 */
           // ret = process_das_process_switch_swset_old(RootData);
          //  process_das_req(socket, dwCMD+1, ret, dwseq);
            break;
        case 0x2843: /* 注册判断 */
           // ret = process_das_process_CenPlt2PuVerifyChallengeCodeReq(RootData);
           // process_das_req(socket, dwCMD+1, ret, dwseq);
            break;
        case 0x3450: /* 设置心跳时间 */
            //ret = process_das_process_CenPlt2PuSetKeepAliveTimeReq(RootData);
           // process_das_req(socket, dwCMD+1, ret, dwseq);
            break;
        case 0x280B: /* 设置报警服务器 */
            //ret = process_das_process_CenPlt2PuSetAlarmCenterReq(RootData);
           // process_das_req(socket, dwCMD+1, ret, dwseq);
            break;
        case 0x2804: /* 心跳 */
           // timenoupdate = 0;
            ret = -1;
            break;
        case 0X3013: /* 升级请求 */
           // upgradeflg = 1;
           // process_das_req(socket, dwCMD+1, ret, dwseq);
            break;
        case 0x3020: /* 校时 */
            //process_das_process_GetServerTimeStampRsp(RootData);
            break;
        case 0X2864: /* 升级http用的 */
           // process_das_process_UpgradeRsp(RootData);
            break;
        case 0X3031: /* 上报统一应答接口 */
            break;
        case 0X3446: /* userid返回 */
            //process_das_process_userid(RootData);
            break;
        case 0x490b: /* 设置屏幕,led,语音 使能开关 */
           // ret= process_das_process_CenPlt2PuSetSwitchEnableReq( RootData);
           // process_das_req(socket, dwCMD+1, ret, dwseq);
            break;
        case 0X4E03: /* 平台向设备应答tvoc,pm25阈值 */
          //  ret = process_das_process_Plt2CenGetThresholdValueRsp(RootData);
            break;
        case 0X4E04: /* 获取传感器即时数据 */
           // ret = process_das_process_getSensorData(RootData, socket, dwseq);
            break;
        case 0X4E07: /* 平台向设备应答电量欠压值  0X4E07*/
           // ret = process_das_process_CenPlt2PuSetLowerPowerRsp(RootData);
            break;
        case 0X498D: /* 平台向设备设置温湿度阈值 */
           // ret = process_das_process_CenPlt2PuSetTempatureHumity(RootData);
            //process_das_req(socket, dwCMD+1, ret, dwseq);
            break;
        default:
           // process_das_req(socket, dwCMD, 0x81, dwseq);
            break;
	}
        ezxml_free(RootData);

  
}
uint8_t net_status=1;
void extend_basefuntion_even_cb(ezdev_sdk_kernel_event* ptr_event, EZDEV_SDK_PTR pUser)
{
    DBG_8195A("ptr_event=%d\r\n",ptr_event->event_type);
    net_status=ptr_event->event_type;
}


void extend_alarm_start_cb(EZDEV_SDK_PTR pUser)
{
}

void extend_alarm_stop_cb(EZDEV_SDK_PTR pUser)
{
}

void extend_alarm_data_route_cb(ezdev_sdk_kernel_submsg* ptr_submsg ,EZDEV_SDK_PTR pUser)
{

     	/**
	 * \brief   constructor of CPerson
	 */
	int dwCMD = 0; 
	if (ptr_submsg == NULL)
	{
		return;
	}
        DBG_8195A("extend_data_route_cb  %s\r\n", ( char *)ptr_submsg->buf);

	dwCMD = ptr_submsg->msg_command_id;
        DBG_8195A("extend_alarm_data_route_cb  %x\r\n", dwCMD);
	int ret = 0;
	switch(dwCMD)
	{
        case 0x1006: /* 开关控制 -- 老接口 */
           // ret = process_das_process_switch_swset_old(RootData);
          //  process_das_req(socket, dwCMD+1, ret, dwseq);
            break;
        default:
           // process_das_req(socket, dwCMD, 0x81, dwseq);
            break;
	}

}

void extend_alarm_even_cb(ezdev_sdk_kernel_event* ptr_event, EZDEV_SDK_PTR pUser)
{
}

void extend_general_function_start_cb(EZDEV_SDK_PTR pUser)
{
}

void extend_general_function_stop_cb(EZDEV_SDK_PTR pUser)
{
}

void extend_general_function_data_route_cb(ezdev_sdk_kernel_submsg* ptr_submsg ,EZDEV_SDK_PTR pUser)
{

     	/**
	 * \brief   constructor of CPerson
	 */
        ezxml_t RootData=NULL;
	int dwCMD = 0;
	if (ptr_submsg == NULL)
	{
		return;
	}
        DBG_8195A("extend_data_route_cb  %s\r\n", ( char *)ptr_submsg->buf);
        RootData = ezxml_parse_str(( char *)ptr_submsg->buf, ptr_submsg->buf_len);
        if (0 != strlen(ezxml_error(RootData)))
        {
            DBG_8195A("ezxml_parse_str error2 %s\r\n", ezxml_error(RootData));
            return;
        }
	dwCMD = ptr_submsg->msg_command_id;

	int ret = 0;
	switch(dwCMD)
	{
        case 0x300f: /* 开关控制 -- 老接口 */
           // ret = process_das_process_switch_swset_old(RootData);
          //  process_das_req(socket, dwCMD+1, ret, dwseq);
            break;
        case 0x2843: /* 注册判断 */
           // ret = process_das_process_CenPlt2PuVerifyChallengeCodeReq(RootData);
           // process_das_req(socket, dwCMD+1, ret, dwseq);
            break;
        case 0x3450: /* 设置心跳时间 */
            //ret = process_das_process_CenPlt2PuSetKeepAliveTimeReq(RootData);
           // process_das_req(socket, dwCMD+1, ret, dwseq);
            break;
        case 0x280B: /* 设置报警服务器 */
            //ret = process_das_process_CenPlt2PuSetAlarmCenterReq(RootData);
           // process_das_req(socket, dwCMD+1, ret, dwseq);
            break;
        case 0x2804: /* 心跳 */
           // timenoupdate = 0;
            ret = -1;
            break;
        case 0X3013: /* 升级请求 */
           // upgradeflg = 1;
           // process_das_req(socket, dwCMD+1, ret, dwseq);
            break;
        case 0x3020: /* 校时 */
            //process_das_process_GetServerTimeStampRsp(RootData);
            break;
        case 0X2864: /* 升级http用的 */
           // process_das_process_UpgradeRsp(RootData);
            break;
        case 0X3031: /* 上报统一应答接口 */
            break;
        case 0X3446: /* userid返回 */
            //process_das_process_userid(RootData);
            break;
        case 0x490b: /* 设置屏幕,led,语音 使能开关 */
           // ret= process_das_process_CenPlt2PuSetSwitchEnableReq( RootData);
           // process_das_req(socket, dwCMD+1, ret, dwseq);
            break;
        case 0X4E03: /* 平台向设备应答tvoc,pm25阈值 */
          //  ret = process_das_process_Plt2CenGetThresholdValueRsp(RootData);
            break;
        case 0X4E04: /* 获取传感器即时数据 */
           // ret = process_das_process_getSensorData(RootData, socket, dwseq);
            break;
        case 0X4E07: /* 平台向设备应答电量欠压值  0X4E07*/
           // ret = process_das_process_CenPlt2PuSetLowerPowerRsp(RootData);
            break;
        case 0X498D: /* 平台向设备设置温湿度阈值 */
           // ret = process_das_process_CenPlt2PuSetTempatureHumity(RootData);
            //process_das_req(socket, dwCMD+1, ret, dwseq);
            break;
        default:
           // process_das_req(socket, dwCMD, 0x81, dwseq);
            break;
	}
        ezxml_free(RootData);
}

void extend_general_function_even_cb(ezdev_sdk_kernel_event* ptr_event, EZDEV_SDK_PTR pUser)
{
}
static int  ReportStatus_alarmReq(void)
{


	//bscJSON_Hooks memoryHook;

	//memoryHook.malloc_fn = malloc;
	//memoryHook.free_fn = free;
	//bscJSON_InitHooks(&memoryHook);
    int             iStatus;
    short           sTestBufLen = 0;
	
	bscJSON *IOTJSObject = NULL, *colorJSObject = NULL;
	char *iot_json = NULL;
	
	if((IOTJSObject = bscJSON_CreateObject()) != NULL) {

		bscJSON_AddItemToObject(IOTJSObject, "devSerial", bscJSON_CreateString("999999998"));
                bscJSON_AddItemToObject(IOTJSObject, "channel", bscJSON_CreateNumber(0));
                bscJSON_AddItemToObject(IOTJSObject, "channelType",bscJSON_CreateNumber(0));
                bscJSON_AddItemToObject(IOTJSObject, "alarmType", bscJSON_CreateString("alarm"));
                bscJSON_AddItemToObject(IOTJSObject, "alarmId", bscJSON_CreateNumber(2));
                bscJSON_AddItemToObject(IOTJSObject, "relationId", bscJSON_CreateNumber(3));
                bscJSON_AddItemToObject(IOTJSObject, "status", bscJSON_CreateNumber(1));
                bscJSON_AddItemToObject(IOTJSObject, "describe", bscJSON_CreateString("tvoc_alarm"));
                bscJSON_AddItemToObject(IOTJSObject, "timestamp", bscJSON_CreateNumber(6));
                bscJSON_AddItemToObject(IOTJSObject, "customType",bscJSON_CreateString("CS-T50"));
                bscJSON_AddItemToObject(IOTJSObject, "customInfo", bscJSON_CreateString("alarm"));


                
                
                iot_json = bscJSON_Print(IOTJSObject);
                sTestBufLen = strlen(iot_json);
                iStatus= send_msg_to_platform(iot_json, sTestBufLen,2000, 0x1005 ,"1000") ; 
		bscJSON_Delete(IOTJSObject);
                free(iot_json);
		
	}
        return iStatus;

}
int ReportStatus_GetServerTimeStampReq(int socket)
{
    int             iStatus;
    short           sTestBufLen = 0;
    ezxml_t         RootData;
    ezxml_t         SendData;
    char            *s;
    char sntemp[64];
    char *pBsdBuf = NULL;

    pBsdBuf = malloc(1400);
  //  HIKbzero((char *)&stHead, sizeof(stHead));
    memset((char *)pBsdBuf,0, 1400);

    SendData = ezxml_new("Request");
    RootData = SendData;
    //HIKbzero(sntemp, sizeof(sntemp));
   // memcpy(sntemp,g_bootparam.prodNo,9);
    ezxml_set_txt(ezxml_add_child(SendData, "DevSerial", 55), "999999998");  
    ezxml_set_txt(ezxml_add_child(SendData, "Authorization", 55), "");
    s = ezxml_toxml(RootData);
    sprintf(pBsdBuf, "<?xml version=\"1.0\" encoding=\"utf-8\"?>%s",s);
    free(s);
    ezxml_free(RootData); 


   // stHead.dwSeq = HIKhtonl(g_dwSeq++);

    sTestBufLen = strlen(pBsdBuf);

    DBG_8195A("\r\n%s\r\n", pBsdBuf);
    
    iStatus= send_msg_to_platform(pBsdBuf, sTestBufLen,1100, 0x301f ,"1000") ; 
    
    //iStatus = HIKsend(socket, pBsdBuf, sTestBufLen, 0 );
    if( iStatus < 0 )
    {
        free(pBsdBuf);
        return iStatus;
    }
    free(pBsdBuf);

    return SUCCESS;
}



void test_task(void *pvParameters)
{

        sdk_thread_sleep(2000);
        //sdk_thread_sleep(5000);

        char *ssid = "OPPO-A37m";
	char *password = "12345678";
	if(wifi_connect(ssid, RTW_SECURITY_WPA2_AES_PSK, password, strlen(ssid), strlen(password), -1, NULL) == RTW_SUCCESS)	
        LwIP_DHCP(0, DHCP_START);
        
        sdk_thread_sleep(4000);
        
        ezdev_sdk_kernel_error sdk_error = ezdev_sdk_kernel_succ;
	ezdev_sdk_kernel_platform_handle kernel_platform_handle;
	memset(&kernel_platform_handle, 0 ,sizeof(kernel_platform_handle));

	kernel_platform_handle.net_work_create = net_create;
	kernel_platform_handle.net_work_connect = net_connect;
	kernel_platform_handle.net_work_read = net_read;
	kernel_platform_handle.net_work_write = net_write;
	kernel_platform_handle.net_work_disconnect = net_disconnect;
	kernel_platform_handle.net_work_destroy = net_destroy;

	kernel_platform_handle.time_creator = Platform_TimerCreater;
	kernel_platform_handle.time_isexpired_bydiff = Platform_TimeIsExpired_Bydiff;
	kernel_platform_handle.time_isexpired = Platform_TimerIsExpired;
	kernel_platform_handle.time_countdownms = Platform_TimerCountdownMS;
	kernel_platform_handle.time_countdown = Platform_TimerCountdown;
	kernel_platform_handle.time_leftms = Platform_TimerLeftMS;
	kernel_platform_handle.time_destroy = Platform_TimeDestroy;

	kernel_platform_handle.sdk_kernel_log = sdk_kernel_logprint;

	kernel_platform_handle.key_value_load = value_load; 
	kernel_platform_handle.key_value_save = value_save;

        
	kernel_platform_handle.thread_mutex_create = sdk_platform_thread_mutex_create;
	kernel_platform_handle.thread_mutex_destroy = sdk_platform_thread_mutex_destroy;
	kernel_platform_handle.thread_mutex_lock = sdk_platform_thread_mutex_lock;
	kernel_platform_handle.thread_mutex_unlock = sdk_platform_thread_mutex_unlock;
        
        
	DBG_8195A("\r\n panlong test_task start SDK version:V1.0\r\n");


	sdk_error = ezdev_sdk_kernel_init("61.153.2.189", 8666, &kernel_platform_handle, event_notice_from_sdk_kernel, DEVINFO_STRING);
	DBG_8195A("panlong test_task ezdev_sdk_kernel_init result:%d\r\n", sdk_error);

	ezdev_sdk_kernel_extend extern_info;
	memset(&extern_info, 0, sizeof(ezdev_sdk_kernel_extend));
#if 1
	extern_info.domain_id = 1100;
	extern_info.pUser = NULL;
	extern_info.ezdev_sdk_kernel_extend_start = extend_basefuntion_start_cb;
	extern_info.ezdev_sdk_kernel_extend_stop = extend_basefuntion_stop_cb;
	extern_info.ezdev_sdk_kernel_extend_data_route = extend_basefuntion_data_route_cb;
	extern_info.ezdev_sdk_kernel_extend_event = extend_basefuntion_even_cb;
	strncpy(extern_info.extend_module_name, "base_function", ezdev_sdk_extend_name_len);
	strncpy(extern_info.extend_module_version, "v1.0.0", ezdev_sdk_extend_name_len);
        sdk_error = ezdev_sdk_kernel_extend_load(&extern_info);
        DBG_8195A("panlong test_task base_function ezdev_sdk_kernel_extend_load result:%d\r\n", sdk_error);
//#else
        

        
        extern_info.domain_id = 2000;
	extern_info.pUser = NULL;
	extern_info.ezdev_sdk_kernel_extend_start = extend_alarm_start_cb;
	extern_info.ezdev_sdk_kernel_extend_stop = extend_alarm_stop_cb;
	extern_info.ezdev_sdk_kernel_extend_data_route = extend_alarm_data_route_cb;
	extern_info.ezdev_sdk_kernel_extend_event = extend_alarm_even_cb;
	strncpy(extern_info.extend_module_name, "alarm_function", ezdev_sdk_extend_name_len);
	strncpy(extern_info.extend_module_version, "v1.0.0", ezdev_sdk_extend_name_len);

	sdk_error = ezdev_sdk_kernel_extend_load(&extern_info);
        
         DBG_8195A("panlong test_task alarm_function ezdev_sdk_kernel_extend_load result:%d\r\n", sdk_error);
 #endif       
        
        
	
	sdk_error = ezdev_sdk_kernel_start();
	DBG_8195A("panlong test_task ezdev_sdk_kernel_start result:%d\r\n", sdk_error);
	int nIndex = 0;
	while(1)
	{
		sdk_error = ezdev_sdk_kernel_yield_user();
		sdk_error = ezdev_sdk_kernel_yield();

 		if (sdk_error == ezdev_sdk_kernel_force_offline)
 		{
                        DBG_8195A("ezdev_sdk_kernel_force_offline....\r\n");
 			ezdev_sdk_kernel_stop();
 		}
                if((nIndex%20)==0)
                    DBG_8195A("\r\npanlong test_task do index:%d, ezdev_sdk_kernel_yield_user result:%d\r\n", nIndex, sdk_error);
                
		nIndex++;
		sdk_thread_sleep(100);
	}

	//DBG_8195A("panlong test_task end SDK version:%s\n", system_get_sdk_version());

	//vTaskDelete(NULL);
}
void ys_task(void)
{


    while(1)
    {
        if(!net_status)
        {
            DBG_8195A("get time....\r\n");
            ReportStatus_GetServerTimeStampReq(0);
            ReportStatus_alarmReq();
        }
        
        vTaskDelay(5000/ portTICK_RATE_MS);  
    }
}
/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void main(void)
{
	/* Initialize log uart and at command service */
//	console_init();	
	ReRegisterPlatformLogUart();

        /* pre-processor of application example */
	pre_example_entry();

	/* wlan intialization */
	wlan_network();
        

        //test_task(NULL);
        xTaskCreate(test_task, "test_task", 2048, NULL, 2, NULL);
        xTaskCreate(ys_task, "ys_task", 1024, NULL, 2, NULL);
        
    	/*Enable Schedule, Start Kernel*/
	vTaskStartScheduler();
}

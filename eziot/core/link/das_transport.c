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

#include "string.h"
#include "das_transport.h"
#include "mkernel_internal_error.h"
#include "base_typedef.h"
#include "sdk_kernel_def.h"
#include "dev_protocol_def.h"
#include "ezdev_sdk_kerne_queuel.h"
#include "bscJSON.h"
#include "MQTTClient.h"
#include "ase_support.h"
#include "ezdev_sdk_kernel_extend.h"
#include "ezdev_sdk_kernel_common.h"
#include "ezdev_sdk_kernel_risk_control.h"
#include "ezdev_sdk_kernel_event.h"
#include "access_domain_bus.h"
#include "utils.h"

EXTERN_QUEUE_FUN(submsg)
EXTERN_QUEUE_FUN(pubmsg_exchange)
EXTERN_QUEUE_FUN(submsg_v3)
EXTERN_QUEUE_FUN(pubmsg_exchange_v3)

EXTERN_QUEUE_BASE_FUN
ASE_SUPPORT_INTERFACE
EZDEV_SDK_KERNEL_EXTEND_INTERFACE
ACCESS_DOMAIN_BUS_INTERFACE
EZDEV_SDK_KERNEL_COMMON_INTERFACE
EZDEV_SDK_KERNEL_RISK_CONTROL_INTERFACE
EZDEV_SDK_KERNEL_EVENT_INTERFACE

MQTTClient g_DasClient;
Network g_DasNetWork;
unsigned char g_sendbuf[ezdev_sdk_send_buf_max];
unsigned char g_readbuf[ezdev_sdk_recv_buf_max];
EZDEV_SDK_UINT32 g_das_transport_seq; ///<	与DAS通信数据包seq
static EZDEV_SDK_BOOL g_is_first_session = EZDEV_SDK_TRUE;

static mkernel_internal_error das_subscribe_revc_topic(ezdev_sdk_kernel *sdk_kernel, EZDEV_SDK_INT8 open);
static mkernel_internal_error das_message_send(ezdev_sdk_kernel *sdk_kernel);
static mkernel_internal_error das_send_pubmsg(ezdev_sdk_kernel *sdk_kernel, ezdev_sdk_kernel_pubmsg *pubmsg);

static mkernel_internal_error serialize_payload_common_v3(EZDEV_SDK_UINT32 msg_seq, unsigned char **output_buf, EZDEV_SDK_UINT16 *output_length)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	char *payload_common_jsonstring = NULL;
	bscJSON *pJsonRoot = NULL;
	do
	{
		pJsonRoot = bscJSON_CreateObject();
		if (NULL == pJsonRoot)
		{
			sdk_error = mkernel_internal_malloc_error;
			break;
		}
		bscJSON_AddNumberToObject(pJsonRoot, "Seq", msg_seq);
		payload_common_jsonstring = bscJSON_PrintUnformatted(pJsonRoot);
		if (NULL == payload_common_jsonstring)
		{
			sdk_error = mkernel_internal_json_format_error;
			break;
		}
		*output_buf = (unsigned char *)payload_common_jsonstring;
		*output_length = strlen(payload_common_jsonstring);
	} while (0);

	if (NULL != pJsonRoot)
	{
		bscJSON_Delete(pJsonRoot);
		pJsonRoot = NULL;
	}
	return sdk_error;
}

static mkernel_internal_error serialize_payload_common(EZDEV_SDK_INT8 msg_type, const char *cmd_version, EZDEV_SDK_UINT32 msg_seq, unsigned char **output_buf, EZDEV_SDK_UINT16 *output_length)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	char *payload_common_jsonstring = NULL;
	bscJSON *pJsonRoot = NULL;
	do
	{
		pJsonRoot = bscJSON_CreateObject();
		if (NULL == pJsonRoot)
		{
			sdk_error = mkernel_internal_malloc_error;
			break;
		}
		bscJSON_AddStringToObject(pJsonRoot, "CmdVer", cmd_version);
		bscJSON_AddNumberToObject(pJsonRoot, "Seq", msg_seq);
		bscJSON_AddNumberToObject(pJsonRoot, "MsgType", msg_type);

		payload_common_jsonstring = bscJSON_PrintUnformatted(pJsonRoot);
		if (NULL == payload_common_jsonstring)
		{
			sdk_error = mkernel_internal_json_format_error;
			break;
		}
		*output_buf = (unsigned char *)payload_common_jsonstring;
		*output_length = strlen(payload_common_jsonstring);
	} while (0);

	if (NULL != pJsonRoot)
	{
		bscJSON_Delete(pJsonRoot);
		pJsonRoot = NULL;
	}
	return sdk_error;
}

static void serialize_short(unsigned char buf[2], EZDEV_SDK_UINT16 src_short)
{
	buf[0] = (unsigned char)(src_short / 256);
	buf[1] = (unsigned char)(src_short % 256);
}

static EZDEV_SDK_UINT16 deserialize_short(unsigned char buf[2])
{
	EZDEV_SDK_UINT16 src_short = buf[0] * 256 + buf[1];
	return src_short;
}

static mkernel_internal_error serialize_lightreginfo(ezdev_sdk_kernel *sdk_kernel, unsigned char **output_buf, EZDEV_SDK_UINT32 *output_length)
{
	char *lightreg_jsstr = NULL;
	EZDEV_SDK_UINT32 jsonstring_len = 0;
	EZDEV_SDK_UINT32 jsonstring_len_padding = 0;
	unsigned char *input_buf = NULL;
	unsigned char *enc_output_buf = NULL;
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	bscJSON *pJsonRoot = NULL;
    char dev_id[ezdev_sdk_devid_len+1]={0};

	do
	{
		pJsonRoot = bscJSON_CreateObject();
		if (NULL == pJsonRoot)
		{
			sdk_error = mkernel_internal_malloc_error;
			break;
		}
		memcpy(dev_id, sdk_kernel->dev_id, ezdev_sdk_devid_len);
		dev_id[ezdev_sdk_devid_len]='\0';
		bscJSON_AddStringToObject(pJsonRoot, "SubSerial", sdk_kernel->dev_info.dev_subserial);
		bscJSON_AddStringToObject(pJsonRoot, "DeviceID", dev_id);
		lightreg_jsstr = bscJSON_PrintUnformatted(pJsonRoot);
		if (lightreg_jsstr == NULL)
		{
			sdk_error = mkernel_internal_json_format_error;
			break;
		}
		jsonstring_len = strlen(lightreg_jsstr);
		jsonstring_len_padding = calculate_padding_len(jsonstring_len);
		input_buf = (unsigned char *)malloc(jsonstring_len_padding);
		enc_output_buf = (unsigned char *)malloc(jsonstring_len_padding);
		if (enc_output_buf == NULL)
		{
			sdk_error = mkernel_internal_malloc_error;
			break;
		}

		if (input_buf == NULL)
		{
			free(enc_output_buf);
			enc_output_buf = NULL;
			sdk_error = mkernel_internal_malloc_error;
			break;
		}
		memset(enc_output_buf, 0, jsonstring_len_padding);
		memset(input_buf, 0, jsonstring_len_padding);
		memcpy(input_buf, lightreg_jsstr, jsonstring_len);
		sdk_error = aes_cbc_128_enc_padding(sdk_kernel->session_key, input_buf, jsonstring_len, jsonstring_len_padding, enc_output_buf, output_length);
		if (sdk_error != mkernel_internal_succ)
		{
			free(enc_output_buf);
			enc_output_buf = NULL;
			break;
		}
		*output_buf = enc_output_buf;
	} while (0);

	if (pJsonRoot != NULL)
	{
		bscJSON_Delete(pJsonRoot);
	}
	if (lightreg_jsstr != NULL)
	{
		free(lightreg_jsstr);
		lightreg_jsstr = NULL;
	}
	if (input_buf != NULL)
	{
		free(input_buf);
		input_buf = NULL;
	}

	return sdk_error;
}

static mkernel_internal_error serialize_devinfo(ezdev_sdk_kernel *sdk_kernel, unsigned char **output_buf, EZDEV_SDK_UINT32 *output_length)
{
	char *devinfo_jsonstring = NULL;
	EZDEV_SDK_UINT32 devinfo_jsonstring_len = 0;
	EZDEV_SDK_UINT32 devinfo_jsonstring_len_padding = 0;
	unsigned char *input_buf = NULL;

	unsigned char *enc_output_buf = NULL;

	mkernel_internal_error sdk_error = mkernel_internal_succ;
	bscJSON *pJsonRoot = NULL;
	char dev_id[ezdev_sdk_devid_len+1]={0};
	do
	{
		pJsonRoot = bscJSON_CreateObject();
		if (NULL == pJsonRoot)
		{
			sdk_error = mkernel_internal_malloc_error;
			break;
		}
        memcpy(dev_id, sdk_kernel->dev_id, ezdev_sdk_devid_len);
		dev_id[ezdev_sdk_devid_len]='\0';
		bscJSON_AddStringToObject(pJsonRoot, "DevSerial", sdk_kernel->dev_info.dev_serial);
		bscJSON_AddStringToObject(pJsonRoot, "SubSerial", sdk_kernel->dev_info.dev_subserial);
		bscJSON_AddStringToObject(pJsonRoot, "FirmwareVersion", sdk_kernel->dev_info.dev_firmwareversion);
		bscJSON_AddStringToObject(pJsonRoot, "DevType", sdk_kernel->dev_info.dev_type);
		bscJSON_AddStringToObject(pJsonRoot, "DevTypeDisplay", sdk_kernel->dev_info.dev_typedisplay);
		bscJSON_AddStringToObject(pJsonRoot, "MAC", sdk_kernel->dev_info.dev_mac);
		bscJSON_AddNumberToObject(pJsonRoot, "Status", sdk_kernel->dev_info.dev_status);
		bscJSON_AddStringToObject(pJsonRoot, "NickName", sdk_kernel->dev_info.dev_nickname);
		bscJSON_AddStringToObject(pJsonRoot, "FirmwareIdentificationCode", sdk_kernel->dev_info.dev_firmwareidentificationcode);
		bscJSON_AddNumberToObject(pJsonRoot, "dev_oeminfo", sdk_kernel->dev_info.dev_oeminfo);
		bscJSON_AddStringToObject(pJsonRoot, "LbsDomain", sdk_kernel->server_info.server_name);
		bscJSON_AddNumberToObject(pJsonRoot, "RegMode", sdk_kernel->reg_mode);
		bscJSON_AddStringToObject(pJsonRoot, "SDKMainVersion", sdk_kernel->szMainVersion);
		bscJSON_AddStringToObject(pJsonRoot, "DeviceID",dev_id);
        
		sdk_error = extend_serialize_sdk_version(pJsonRoot);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}
		devinfo_jsonstring = bscJSON_PrintUnformatted(pJsonRoot);
		if (devinfo_jsonstring == NULL)
		{
			sdk_error = mkernel_internal_json_format_error;
			break;
		}
		devinfo_jsonstring_len = strlen(devinfo_jsonstring);
		devinfo_jsonstring_len_padding = calculate_padding_len(devinfo_jsonstring_len);
		input_buf = (unsigned char *)malloc(devinfo_jsonstring_len_padding);
		enc_output_buf = (unsigned char *)malloc(devinfo_jsonstring_len_padding);
		if (enc_output_buf == NULL)
		{
			sdk_error = mkernel_internal_malloc_error;
			break;
		}

		if (input_buf == NULL)
		{
			free(enc_output_buf);
			enc_output_buf = NULL;
			sdk_error = mkernel_internal_malloc_error;
			break;
		}

		memset(enc_output_buf, 0, devinfo_jsonstring_len_padding);
		memset(input_buf, 0, devinfo_jsonstring_len_padding);
		memcpy(input_buf, devinfo_jsonstring, devinfo_jsonstring_len);

		sdk_error = aes_cbc_128_enc_padding(sdk_kernel->session_key,
											(unsigned char *)input_buf, devinfo_jsonstring_len, devinfo_jsonstring_len_padding,
											enc_output_buf, output_length);
		if (sdk_error != mkernel_internal_succ)
		{
			free(enc_output_buf);
			enc_output_buf = NULL;
			break;
		}
		*output_buf = enc_output_buf;
	} while (0);

	if (pJsonRoot != NULL)
	{
		bscJSON_Delete(pJsonRoot);
		pJsonRoot = NULL;
	}
	if (devinfo_jsonstring != NULL)
	{
		free(devinfo_jsonstring);
		devinfo_jsonstring = NULL;
	}
	if (input_buf != NULL)
	{
		free(input_buf);
		input_buf = NULL;
	}

	return sdk_error;
}

static mkernel_internal_error deserialize_common_v3(unsigned char *common_buf, EZDEV_SDK_UINT16 common_buf_len, ezdev_sdk_kernel_submsg_v3 *ptr_submsg)
{
	/**
	* \brief  解析通用协议体
	*/
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	bscJSON *json_item = NULL;
	bscJSON *json_seq_item = NULL;


	do
	{
		json_item = bscJSON_Parse((const char *)common_buf);
		if (json_item == NULL)
		{
			ezdev_sdk_kernel_log_debug(mkernel_internal_json_parse_error, 0, "deserialize_common Parse json error");
			sdk_error = mkernel_internal_json_parse_error;
			break;
		}
		json_seq_item = bscJSON_GetObjectItem(json_item, "Seq");
	
		if (json_seq_item == NULL)
		{
			ezdev_sdk_kernel_log_debug(mkernel_internal_json_parse_error, 0, "deserialize_common Parse seq miss feild error");
			sdk_error = mkernel_internal_json_parse_error;
			break;
		}

		if (json_seq_item->type != bscJSON_Number)
		{
			ezdev_sdk_kernel_log_debug(mkernel_internal_json_parse_error, 0, "deserialize_common Parse BusiVer or CmdVer type error");
			sdk_error = mkernel_internal_json_parse_error;
			break;
		}
		ptr_submsg->msg_seq = json_seq_item->valueint;

	} while (0);

	if (json_item != NULL)
	{
		bscJSON_Delete(json_item);
		json_item = NULL;
	}

	return sdk_error;
}


static mkernel_internal_error deserialize_common(unsigned char *common_buf, EZDEV_SDK_UINT16 common_buf_len, ezdev_sdk_kernel_submsg *ptr_submsg)
{
	/**
	* \brief  解析通用协议体
	*/
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	bscJSON *json_item = NULL;
	bscJSON *json_seq_item = NULL;
	bscJSON *json_cmdver_item = NULL;

	do
	{
		json_item = bscJSON_Parse((const char *)common_buf);
		if (json_item == NULL)
		{
			ezdev_sdk_kernel_log_debug(mkernel_internal_json_parse_error, 0, "deserialize_common Parse json error");
			sdk_error = mkernel_internal_json_parse_error;
			break;
		}
		json_seq_item = bscJSON_GetObjectItem(json_item, "Seq");
		json_cmdver_item = bscJSON_GetObjectItem(json_item, "CmdVer");
		if (json_cmdver_item == NULL || json_seq_item == NULL)
		{
			ezdev_sdk_kernel_log_debug(mkernel_internal_json_parse_error, 0, "deserialize_common Parse BusiVer or CmdVer miss feild error");
			sdk_error = mkernel_internal_json_parse_error;
			break;
		}

		if (json_seq_item->type != bscJSON_Number ||
			json_cmdver_item->type != bscJSON_String || NULL == json_cmdver_item->valuestring)
		{
			ezdev_sdk_kernel_log_debug(mkernel_internal_json_parse_error, 0, "deserialize_common Parse BusiVer or CmdVer type error");
			sdk_error = mkernel_internal_json_parse_error;
			break;
		}

		if (strlen(json_cmdver_item->valuestring) >= version_max_len)
		{
			strncpy(ptr_submsg->command_ver, json_cmdver_item->valuestring, version_max_len - 1);
		}
		else
		{
			strncpy(ptr_submsg->command_ver, json_cmdver_item->valuestring, strlen(json_cmdver_item->valuestring));
		}

		ptr_submsg->msg_seq = json_seq_item->valueint;

	} while (0);

	if (json_item != NULL)
	{
		bscJSON_Delete(json_item);
		json_item = NULL;
	}
	// 	if (comon_buf != NULL)
	// 	{
	// 		free(comon_buf);
	// 		comon_buf = NULL;
	// 	}
	return sdk_error;
}

static void handle_sub_msg_v3(ezdev_sdk_kernel_submsg_v3 *ptr_submsg)
{
	EZDEV_SDK_BOOL is_delete = EZDEV_SDK_TRUE;
	mkernel_internal_error kernel_error = mkernel_internal_succ;
	kernel_error = push_queue_submsg_v3(ptr_submsg);
	if (kernel_error != mkernel_internal_succ)
	{
		ezdev_sdk_kernel_log_debug(kernel_error, 0, "push_queue_submsg v3 error,module:%s, seq:%d", ptr_submsg->module, ptr_submsg->msg_seq);
	}
	else
	{
		is_delete = EZDEV_SDK_FALSE;
	}

	if (is_delete)
	{
		if (ptr_submsg->buf != NULL)
		{
			free(ptr_submsg->buf);
			ptr_submsg->buf = NULL;
		}

		free(ptr_submsg);
		ptr_submsg = NULL;
	}
}

static void handle_sub_msg(ezdev_sdk_kernel_submsg *ptr_submsg)
{
	EZDEV_SDK_BOOL is_delete = EZDEV_SDK_TRUE;
	mkernel_internal_error kernel_error = mkernel_internal_succ;

	if (ptr_submsg->msg_domain_id == DAS_CMD_DOMAIN)
	{
		kernel_error = access_domain_bus_handle(ptr_submsg);
	}
	else if (ptr_submsg->msg_domain_id == DAS_CMD_COMMON_FUN || ptr_submsg->msg_command_id == DAS_CMD_PU2CENPLTUPGRADERSP)
	{
		//通用领域
		if (common_module_bus_handle(ptr_submsg))
		{
			kernel_error = push_queue_submsg(ptr_submsg);
			if (kernel_error != mkernel_internal_succ)
			{
				ezdev_sdk_kernel_log_debug(kernel_error, 0, "handle_sub_msg push_queue_submsg error,demain:%d, cmd:%d", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id);
			}
			else
			{
				is_delete = EZDEV_SDK_FALSE;
			}
		}
	}
	else
	{
		kernel_error = push_queue_submsg(ptr_submsg);
		if (kernel_error != mkernel_internal_succ)
		{
			ezdev_sdk_kernel_log_debug(kernel_error, 0, "handle_sub_msg push_queue_submsg error,demain:%d, cmd:%d", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id);
		}
		else
		{
			is_delete = EZDEV_SDK_FALSE;
		}
	}
	if (is_delete)
	{
		if (ptr_submsg->buf != NULL)
		{
			free(ptr_submsg->buf);
			ptr_submsg->buf = NULL;
		}

		free(ptr_submsg);
		ptr_submsg = NULL;
	}
}

static mkernel_internal_error ezdev_parse_topic(ezdev_sdk_kernel_submsg_v3* ptr_submsg, char* topic, char* find_str)
{
	char* ptemp1 = NULL;
	char* ptemp2 = NULL;
	int temp_len = 0;
	int find_str_len = 0;
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	if(NULL== ptr_submsg||NULL == topic||NULL == find_str)
	{
       return  mkernel_internal_input_param_invalid;
	}
	do
	{
		find_str_len = strlen(find_str);
		ptemp1 = strstr(topic, find_str);
		if(!ptemp1)
		{
			ezdev_sdk_kernel_log_error(mkernel_internal_platform_appoint_error, 0, "ezdev_parse_topic, not find:%s \n", find_str);
			sdk_error = mkernel_internal_platform_appoint_error;
			break;
		}
		ptemp2 = strrchr(ptemp1, '/');
        if(!ptemp2)
		{
			sdk_error = mkernel_internal_platform_appoint_error;
			break;
		}
		temp_len = ptemp2 - ptemp1 - find_str_len;
		if(temp_len <=0|| temp_len > sizeof(ptr_submsg->method))
		{
			ezdev_sdk_kernel_log_error(mkernel_internal_platform_appoint_error, 0, "ezdev_parse_topic temp_len, calculate err:%d \n", temp_len);
			sdk_error = mkernel_internal_platform_appoint_error;
			break;
		}
		strncpy(ptr_submsg->msg_type, ptemp2 + 1, sizeof(ptr_submsg->msg_type)-1);
		strncpy(ptr_submsg->method, ptemp1 + find_str_len, temp_len);

	}while(0);
    
	return sdk_error;
}

static void das_message_receive_v3(MessageData *msg_data)
{
	/**
	* \brief  topic /{领域编号}/{指令编号}
	*/
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	ezdev_sdk_kernel_submsg_v3 *ptr_submsg = NULL;
	EZDEV_SDK_INT32 division_num = 0;
	unsigned char *output_buf = NULL;
	EZDEV_SDK_UINT32 output_buf_len = 0;

	EZDEV_SDK_UINT16 common_len = 0;
	unsigned char common_len_buf[2];
	memset(common_len_buf, 0, 2);
	char find_str[32] = {0};
	char msg_topic[512];
	char dev_serial[ezdev_sdk_devserial_maxlen];
	memset(msg_topic, 0, 512);
	memset(dev_serial, 0, ezdev_sdk_devserial_maxlen);
	do
	{
		ptr_submsg = (ezdev_sdk_kernel_submsg_v3 *)malloc(sizeof(ezdev_sdk_kernel_submsg_v3));
		if (ptr_submsg == NULL)
		{
			ezdev_sdk_kernel_log_debug(mkernel_internal_malloc_error, 0, "das_message_receive_v3 mallc submsg error ");
			sdk_error = mkernel_internal_malloc_error;
			break;
		}
		memset(ptr_submsg, 0, sizeof(ezdev_sdk_kernel_submsg_v3));

		if (msg_data->topicName->lenstring.len >= 512)
		{
			ezdev_sdk_kernel_log_error(mkernel_internal_platform_appoint_error, 0, "  recv a das v3 msg which size is too len:%d", msg_data->topicName->lenstring.len);
			sdk_error = mkernel_internal_platform_appoint_error;
			break;
		}
		strncpy(msg_topic, msg_data->topicName->lenstring.data, msg_data->topicName->lenstring.len);

		ezdev_sdk_kernel_log_debug(0, 0, "das_message_receive_v3 msg_topic: %s \n", msg_topic);

        division_num = sscanf(msg_topic, "/iot/%72[^/]/%72[^/]/%64[^-]-%64[^/]/%16[^/]/", dev_serial, &ptr_submsg->sub_serial, &ptr_submsg->resource_id, &ptr_submsg->resource_type, \
            &ptr_submsg->module);
        if (division_num != 5)
        {
            ezdev_sdk_kernel_log_error(mkernel_internal_platform_appoint_error, 0, " decode common topic err :%s\n", msg_topic);
            sdk_error = mkernel_internal_platform_appoint_error;
            break;
        }
		snprintf(find_str, 32, "%s/", ptr_submsg->module);
        if (0 == strcmp(ptr_submsg->module, "model"))
        {
            division_num = sscanf(msg_topic, "/iot/%72[^/]/%72[^/]/%64[^-]-%64[^/]/model/%64[^/]/%32[^/]/%s", dev_serial, &ptr_submsg->sub_serial, &ptr_submsg->resource_id, &ptr_submsg->resource_type, \
                                 &ptr_submsg->method, &ptr_submsg->msg_type, &ptr_submsg->ext_msg);
            if (division_num != 7)
            {
                ezdev_sdk_kernel_log_error(mkernel_internal_platform_appoint_error, 0, "decode model topic error :%s\n", msg_topic);
                sdk_error = mkernel_internal_platform_appoint_error;
                break;
            }
        }
        else 
        {
		   sdk_error = ezdev_parse_topic(ptr_submsg, msg_topic, find_str);
		   if(mkernel_internal_succ !=sdk_error)
		   {
			   break;
		   }
        }
		/**
		 * \brief   将整个报文解密
		 */
		output_buf = (unsigned char *)malloc(msg_data->message->payloadlen);
		if (NULL == output_buf)
		{
			ezdev_sdk_kernel_log_debug(mkernel_internal_malloc_error, 0, "receive_v3 malloc err,module:%s\n", ptr_submsg->module);
			sdk_error = mkernel_internal_malloc_error;
			break;
		}
		memset(output_buf, 0, msg_data->message->payloadlen);

		sdk_error = aes_cbc_128_dec_padding(get_ezdev_sdk_kernel()->session_key, (unsigned char *)msg_data->message->payload, msg_data->message->payloadlen, output_buf, &output_buf_len);
		if (sdk_error != mkernel_internal_succ)
		{
			ezdev_sdk_kernel_log_debug(sdk_error, 0, "receive_v3 aes_128_dec_padding err,module:%s, seq:%d", ptr_submsg->module, ptr_submsg->msg_seq);
			break;
		}
        
		memcpy(common_len_buf, output_buf, 2);
		common_len = deserialize_short(common_len_buf);
		if (common_len >= output_buf_len)
		{
			ezdev_sdk_kernel_log_debug(mkernel_internal_rev_invalid_packet, 0, "v3  common len decode err, module:%s, seq:%d",  ptr_submsg->module, ptr_submsg->msg_seq);
			sdk_error = mkernel_internal_rev_invalid_packet;
			break;
		}
		sdk_error = deserialize_common_v3((unsigned char *)output_buf + 2, common_len, ptr_submsg);
		if (sdk_error != mkernel_internal_succ)
		{
			ezdev_sdk_kernel_log_debug(sdk_error, 0, "v3 deserialize_common err,module:%s, seq:%d", ptr_submsg->module, ptr_submsg->msg_seq);
			break;
		}
		if (output_buf_len - 2 - common_len > 0)
		{
			memmove(output_buf, output_buf + 2 + common_len, output_buf_len - 2 - common_len);
			memset(output_buf + output_buf_len - 2 - common_len, 0, 2 + common_len);
			ptr_submsg->buf = output_buf;
			ptr_submsg->buf_len = output_buf_len - 2 - common_len;
		}
		else
		{
			/**
			* \brief   数据为空
			*/
		    //数据为空包的时候不能直接返回空指针,这里malloc一个字节的空间
			ptr_submsg->buf = malloc(sizeof(char));
			if(NULL == ptr_submsg->buf)
			{  
				ezdev_sdk_kernel_log_error(mkernel_internal_malloc_error, 0, "parse das msg context is empty,malloc err");
				sdk_error = mkernel_internal_malloc_error;
				break;
			}
			memset(ptr_submsg->buf, 0 , sizeof(char));
			ptr_submsg->buf_len = sizeof(char);
			ezdev_sdk_kernel_log_debug(sdk_error, 0, "Recv v3 empty context  module:%s, seq:%d", ptr_submsg->module, ptr_submsg->msg_seq);

			free(output_buf);
			output_buf = NULL;
		}
		ezdev_sdk_kernel_log_debug(sdk_error, sdk_error, "das_message_receive_v3 payloadlen:%lu, seq:%d", msg_data->message->payloadlen, ptr_submsg->msg_seq);
	} while (0);

	/**
	* \brief   成功返回
	*/
	if (sdk_error == mkernel_internal_succ)
	{
		handle_sub_msg_v3(ptr_submsg);
		return;
	}

	if (ptr_submsg != NULL)
	{
		free(ptr_submsg);
		ptr_submsg = NULL;
	}
	if (output_buf != NULL)
	{
		free(output_buf);
		output_buf = NULL;
	}
}


void das_message_receive_ex(MessageData* msg_data)
{
    ezdev_sdk_kernel_error sdk_error = ezdev_sdk_kernel_succ;
    ezdev_sdk_kernel_submsg* ptr_submsg = NULL;
    EZDEV_SDK_INT32 division_num = 0;
    EZDEV_SDK_UINT16 common_len = 0;
    unsigned char *output_buf = NULL;
    EZDEV_SDK_UINT32 output_buf_len = 0;
    char msg_topic[128];
    unsigned char common_len_buf[2];
    char dev_serial[ezdev_sdk_devserial_maxlen];

    memset(msg_topic, 0, 128);
    memset(common_len_buf, 0, 2);
    memset(dev_serial, 0, ezdev_sdk_devserial_maxlen);

    if(msg_data == NULL)
	{
		goto fail;
	}
    ptr_submsg = (ezdev_sdk_kernel_submsg*)malloc(sizeof(ezdev_sdk_kernel_submsg));
    if (ptr_submsg == NULL)
    {
        ezdev_sdk_kernel_log_debug(ezdev_sdk_kernel_memory, 0, "das_message_receive_ex mallc submsg error ");
        sdk_error = ezdev_sdk_kernel_memory;
        goto fail;
    }
    memset(ptr_submsg, 0, sizeof(ezdev_sdk_kernel_submsg));
	if (msg_data->topicName->lenstring.len >= 128)
	{
		ezdev_sdk_kernel_log_error(ezdev_sdk_kernel_data_len_range, 0, "das_message_receive_ex recv a msg which size is too len:%d\n", msg_data->topicName->lenstring.len);
		sdk_error = ezdev_sdk_kernel_data_len_range;
		goto fail;
	}

	strncpy(msg_topic, msg_data->topicName->lenstring.data, msg_data->topicName->lenstring.len);

	division_num = sscanf(msg_topic, "/%16[^/]/%d/%d", dev_serial, &ptr_submsg->msg_domain_id, &ptr_submsg->msg_command_id);
	if (division_num != 3)
	{
		ezdev_sdk_kernel_log_error(ezdev_sdk_kernel_data_len_range, 0, "das_message_receive_ex decode topicName error :%s\n", msg_topic);
		sdk_error = ezdev_sdk_kernel_data_len_range;
		goto fail;
	}
	output_buf = (unsigned char *)malloc(msg_data->message->payloadlen);
	if ( NULL == output_buf )
	{
		ezdev_sdk_kernel_log_debug(ezdev_sdk_kernel_memory, 0, "das_message_receive_ex malloc error,domain:%d, cmd:%d, lenstring len:%d \n", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id, msg_data->topicName->lenstring.len);
		sdk_error = ezdev_sdk_kernel_memory;
		goto fail;
	}
	memset(output_buf, 0, msg_data->message->payloadlen);

	sdk_error = aes_cbc_128_dec_padding(get_ezdev_sdk_kernel()->session_key, (unsigned char*)msg_data->message->payload, msg_data->message->payloadlen, output_buf, &output_buf_len);
	if (sdk_error != ezdev_sdk_kernel_succ)
	{
		ezdev_sdk_kernel_log_debug(sdk_error, 0, "das_message_receive_ex aes_cbc_128_dec_padding error,domain:%d, cmd:%d", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id);
		goto fail;
	}

	memcpy(common_len_buf, output_buf, 2);
	common_len = deserialize_short(common_len_buf);
	if (common_len >= output_buf_len)
	{
		ezdev_sdk_kernel_log_debug(ezdev_sdk_kernel_data_len_range, 0, "das_message_receive_ex decode error common len, domain:%d, cmd:%d", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id);
		sdk_error = ezdev_sdk_kernel_data_len_range;
		goto fail;
	}
	sdk_error = deserialize_common((unsigned char*)output_buf + 2, common_len, ptr_submsg);
	if (sdk_error != ezdev_sdk_kernel_succ)
	{
		ezdev_sdk_kernel_log_debug(sdk_error, 0, "das_message_receive_ex deserialize_common error,domain:%d, cmd:%d", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id);
		goto fail;
	}
	if (output_buf_len - 2 - common_len > 0)
	{
		memmove(output_buf, output_buf + 2 + common_len, output_buf_len - 2 - common_len);
		memset(output_buf + output_buf_len - 2 - common_len, 0, 2 + common_len);
		ptr_submsg->buf = output_buf;
		ptr_submsg->buf_len = output_buf_len - 2 - common_len;
	}
	else
	{
		ptr_submsg->buf = NULL;
		ptr_submsg->buf_len = 0;

		free(output_buf);
		output_buf = NULL;
	}
	ezdev_sdk_kernel_log_debug(sdk_error, sdk_error, "das_message_receive_ex msg_topic:%s, seq:%d\n", msg_topic, ptr_submsg->msg_seq);
    
    if (sdk_error == ezdev_sdk_kernel_succ)
    {
        if (ptr_submsg->msg_domain_id == 1001)
        {
            common_module_bus_handle(ptr_submsg);
        }
    	ezdev_sdk_kernel_domain_info* kernel_domain = extend_get(ptr_submsg->msg_domain_id);

        if (kernel_domain)
        {
            kernel_domain->kernel_extend.ezdev_sdk_kernel_extend_data_route(ptr_submsg, kernel_domain->kernel_extend.pUser);
        }
        else
        {
            ezdev_sdk_kernel_log_error(0,0,"das_message_receive_ex find domain error %d\n", ptr_submsg->msg_domain_id);
        }
    }
fail:
    ezdev_sdk_kernel_log_error(sdk_error, sdk_error,"das_message_receive_ex end\n");

	if (ptr_submsg != NULL)
	{
		free(ptr_submsg);
		ptr_submsg = NULL;
	}
	if (output_buf != NULL)
	{
		free(output_buf);
		output_buf = NULL;
	}
}


static void das_message_receive(MessageData *msg_data)
{
	/**
	* \brief  topic /{领域编号}/{指令编号}
	*/
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	ezdev_sdk_kernel_submsg *ptr_submsg = NULL;
	EZDEV_SDK_INT32 division_num = 0;
	EZDEV_SDK_UINT16 common_len = 0;
	unsigned char *output_buf = NULL;
	EZDEV_SDK_UINT32 output_buf_len = 0;
	char msg_topic[128];
	unsigned char common_len_buf[2];
	char dev_serial[ezdev_sdk_devserial_maxlen];

	memset(msg_topic, 0, 128);
	memset(common_len_buf, 0, 2);
	memset(dev_serial, 0, ezdev_sdk_devserial_maxlen);
	do
	{
		ptr_submsg = (ezdev_sdk_kernel_submsg *)malloc(sizeof(ezdev_sdk_kernel_submsg));
		if (ptr_submsg == NULL)
		{
			ezdev_sdk_kernel_log_debug(mkernel_internal_malloc_error, 0, "das_message_receive mallc submsg error ");
			sdk_error = mkernel_internal_malloc_error;
			break;
		}
		memset(ptr_submsg, 0, sizeof(ezdev_sdk_kernel_submsg));

		if (msg_data->topicName->lenstring.len >= 128)
		{
			ezdev_sdk_kernel_log_error(mkernel_internal_platform_appoint_error, 0, "das_message_receive recv a msg which size is too len:%d\n", msg_data->topicName->lenstring.len);
			sdk_error = mkernel_internal_platform_appoint_error;
			break;
		}

		strncpy(msg_topic, msg_data->topicName->lenstring.data, msg_data->topicName->lenstring.len);
		division_num = sscanf(msg_topic, "/%72[^/]/%d/%d", dev_serial, &ptr_submsg->msg_domain_id, &ptr_submsg->msg_command_id);
		if (division_num != 3)
		{
			ezdev_sdk_kernel_log_error(mkernel_internal_platform_appoint_error, 0, "das_message_receive decode topicName error :%s", msg_topic);
			sdk_error = mkernel_internal_platform_appoint_error;
			break;
		}
		
		/**
		 * \brief   将整个报文解密
		 */
		output_buf = (unsigned char *)malloc(msg_data->message->payloadlen);
		if (NULL == output_buf)
		{
			ezdev_sdk_kernel_log_debug(mkernel_internal_malloc_error, 0, "das_message_receive mallc error,domain:%d, cmd:%d, len:%d\n", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id, msg_data->topicName->lenstring.len);
			sdk_error = mkernel_internal_malloc_error;
			break;
		}
		memset(output_buf, 0, msg_data->message->payloadlen);

		sdk_error = aes_cbc_128_dec_padding(get_ezdev_sdk_kernel()->session_key, (unsigned char *)msg_data->message->payload, msg_data->message->payloadlen, output_buf, &output_buf_len);
		if (sdk_error != mkernel_internal_succ)
		{
			ezdev_sdk_kernel_log_debug(sdk_error, 0, "das_message_receive aes_cbc_128_dec_padding error,domain:%d, cmd:%d\n", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id);
			break;
		}
		memcpy(common_len_buf, output_buf, 2);
		common_len = deserialize_short(common_len_buf);
		if (common_len >= output_buf_len)
		{
			ezdev_sdk_kernel_log_debug(mkernel_internal_rev_invalid_packet, 0, "das_message_receive decode error common len, domain:%d, cmd:%d\n", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id);
			sdk_error = mkernel_internal_rev_invalid_packet;
			break;
		}
		sdk_error = deserialize_common((unsigned char *)output_buf + 2, common_len, ptr_submsg);
		if (sdk_error != mkernel_internal_succ)
		{
			ezdev_sdk_kernel_log_debug(sdk_error, 0, "das_message_receive deserialize_common error,domain:%d, cmd:%d\n", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id);
			break;
		}
		if (output_buf_len - 2 - common_len > 0)
		{
			memmove(output_buf, output_buf + 2 + common_len, output_buf_len - 2 - common_len);
			memset(output_buf + output_buf_len - 2 - common_len, 0, 2 + common_len);
			ptr_submsg->buf = output_buf;
			ptr_submsg->buf_len = output_buf_len - 2 - common_len;
		}
		else
		{
			//数据为空包的时候不能直接返回空指针,这里malloc一个字节的空间
			ptr_submsg->buf = malloc(sizeof(char));
			if(NULL == ptr_submsg->buf)
			{  
				ezdev_sdk_kernel_log_error(mkernel_internal_malloc_error, 0, "parse das msg context is empty,malloc err\n");
				sdk_error = mkernel_internal_malloc_error;
				break;
			}
			memset(ptr_submsg->buf, 0 , sizeof(char));
			ptr_submsg->buf_len = sizeof(char);
			ezdev_sdk_kernel_log_debug(sdk_error, 0, "Recv v2 empty context, domain_id:%d, cmd_id:%d, seq:%d\n", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id, ptr_submsg->msg_seq);

			free(output_buf);
			output_buf = NULL;
		}
		ezdev_sdk_kernel_log_debug(sdk_error, sdk_error, "das_message_receive msg_topic:%s, seq:%d\n", msg_topic, ptr_submsg->msg_seq);
	} while (0);

	/**
	* \brief   成功返回
	*/
	if (sdk_error == mkernel_internal_succ)
	{
		handle_sub_msg(ptr_submsg);
		return;
	}

	if (ptr_submsg != NULL)
	{
		free(ptr_submsg);
		ptr_submsg = NULL;
	}
	if (output_buf != NULL)
	{
		free(output_buf);
		output_buf = NULL;
	}
}


static mkernel_internal_error das_send_pubmsg_v3(ezdev_sdk_kernel *sdk_kernel, ezdev_sdk_kernel_pubmsg_v3 *pubmsg)
{
	MQTTMessage mqtt_msg;
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	EZDEV_SDK_INT32 mqtt_result_code = 0;
	unsigned char *common_output_buf = NULL;
	EZDEV_SDK_UINT32 enc_output_buf_len = 0;
	unsigned char *payload_buf = NULL;
	unsigned char *payload_buf_enc = NULL;
	EZDEV_SDK_UINT32 payload_buf_len = 0;
	EZDEV_SDK_UINT16 common_output_buf_len = 0;
	unsigned char common_len_serialize_buf[2];
	char dev_serial[ezdev_sdk_devserial_maxlen] = {0};
	char dev_subserial[ezdev_sdk_devserial_maxlen] = {0};
	char publish_topic[512] ={0};
	do
	{
		memset(common_len_serialize_buf, 0, 2);
		strncpy(dev_serial, sdk_kernel->dev_info.dev_subserial, ezdev_sdk_devserial_maxlen - 1);
		if(strlen(pubmsg->sub_serial) > 0)
		{
			strncpy(dev_subserial, pubmsg->sub_serial, ezdev_sdk_devserial_maxlen - 1);
		}
		else
		{
			strncpy(dev_subserial, "global", strlen("global") + 1);
		}
		if( 0 == strcmp(pubmsg->module, "model"))
		{
			snprintf(publish_topic, 512, "/iot/%s/%s/%s-%s/model/%s/%s/%s", dev_serial, dev_subserial, pubmsg->resource_id,\
			        pubmsg->resource_type, pubmsg->method, pubmsg->msg_type, pubmsg->ext_msg);
		}
		else
		{
			snprintf(publish_topic, 512, "/iot/%s/%s/%s-%s/%s/%s/%s", dev_serial, dev_subserial, pubmsg->resource_id,\
			        pubmsg->resource_type, pubmsg->module, pubmsg->method, pubmsg->msg_type);
		}
		ezdev_sdk_kernel_log_debug(0, 0, "publish topic:%s\n", publish_topic);
		memset(&mqtt_msg, 0, sizeof(mqtt_msg));
		mqtt_msg.qos = pubmsg->msg_qos;
		mqtt_msg.retained = 1; //平台不关心
		mqtt_msg.dup = 0;
		sdk_error = serialize_payload_common_v3(pubmsg->msg_seq, &common_output_buf, &common_output_buf_len);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}
		payload_buf_len = calculate_padding_len(common_output_buf_len + 2 + pubmsg->msg_body_len);
		payload_buf = (unsigned char *)malloc(payload_buf_len);
		payload_buf_enc = (unsigned char *)malloc(payload_buf_len);
		if (payload_buf == NULL || payload_buf_enc == NULL)
		{
			ezdev_sdk_kernel_log_error(mkernel_internal_malloc_error, 0, "malloc payload len error\n");
			sdk_error = mkernel_internal_malloc_error;
			break;
		}
		memset(payload_buf, 0, payload_buf_len);
		memset(payload_buf_enc, 0, payload_buf_len);
		serialize_short(common_len_serialize_buf, common_output_buf_len);
		memcpy(payload_buf, common_len_serialize_buf, 2);
		memcpy(payload_buf + 2, common_output_buf, common_output_buf_len);
		memcpy(payload_buf + 2 + common_output_buf_len, pubmsg->msg_body, pubmsg->msg_body_len);

		sdk_error = aes_cbc_128_enc_padding(sdk_kernel->session_key,payload_buf, common_output_buf_len + 2 + pubmsg->msg_body_len, payload_buf_len, payload_buf_enc, &enc_output_buf_len);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}
		mqtt_msg.payload = (void *)payload_buf_enc;
		mqtt_msg.payloadlen = enc_output_buf_len;
		mqtt_result_code = MQTTPublish(&g_DasClient, publish_topic, &mqtt_msg);

		if (mqtt_result_code == MQTTPACKET_BUFFER_TOO_SHORT)
		{
			ezdev_sdk_kernel_log_error(mkernel_internal_call_mqtt_buffer_too_short, mqtt_result_code, "mqtt buffer too short\n");
			sdk_error = mkernel_internal_call_mqtt_buffer_too_short;
			break;
		}

		if (mqtt_result_code != 0)
		{
			ezdev_sdk_kernel_log_error(mkernel_internal_call_mqtt_pub_error, mqtt_result_code, "mqtt publish error\n");
			sdk_error = mkernel_internal_call_mqtt_pub_error;
			break;
		}

	} while (0);
    
	if (common_output_buf)
	{
		free(common_output_buf);
		common_output_buf = NULL;
	}
	if (payload_buf)
	{
		free(payload_buf);
		payload_buf = NULL;
	}
	if (payload_buf_enc)
	{
		free(payload_buf_enc);
		payload_buf_enc = NULL;
	}
	return sdk_error;
}


static mkernel_internal_error das_send_pubmsg(ezdev_sdk_kernel *sdk_kernel, ezdev_sdk_kernel_pubmsg *pubmsg)
{
	MQTTMessage mqtt_msg;
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	EZDEV_SDK_INT32 mqtt_result_code = 0;
	unsigned char *common_output_buf = NULL;
	EZDEV_SDK_UINT32 enc_output_buf_len = 0;
	unsigned char *payload_buf = NULL;
	unsigned char *payload_buf_enc = NULL;
	EZDEV_SDK_UINT32 payload_buf_len = 0;
	EZDEV_SDK_UINT16 common_output_buf_len = 0;
	EZDEV_SDK_INT8 msg_type = 0;

	unsigned char common_len_serialize_buf[2];
	char publish_topic[128];
	memset(common_len_serialize_buf, 0, 2);
	memset(publish_topic, 0, 128);
	snprintf(publish_topic, 128, "/%d/%d", pubmsg->msg_domain_id, pubmsg->msg_command_id);

	memset(&mqtt_msg, 0, sizeof(mqtt_msg));
	mqtt_msg.qos = pubmsg->msg_qos;
	mqtt_msg.retained = 1; //平台不关心
	mqtt_msg.dup = 0;	  //0 非重试  1 重试

	if (pubmsg->msg_response == 0)
	{
		msg_type = ezdev_sdk_msg_type_req;
	}
	else
	{
		msg_type = ezdev_sdk_msg_type_rsp;
	}
	do
	{
		sdk_error = serialize_payload_common(msg_type, pubmsg->command_ver, pubmsg->msg_seq, &common_output_buf, &common_output_buf_len);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}
		payload_buf_len = calculate_padding_len(common_output_buf_len + 2 + pubmsg->msg_body_len);
		// = common_output_buf_len + 2 + pubmsg->msg_body_len;
		payload_buf = (unsigned char *)malloc(payload_buf_len);
		payload_buf_enc = (unsigned char *)malloc(payload_buf_len);
		if (payload_buf == NULL || payload_buf_enc == NULL)
		{
			ezdev_sdk_kernel_log_error(mkernel_internal_malloc_error, 0, "malloc payload error\n");
			sdk_error = mkernel_internal_malloc_error;
			break;
		}
		memset(payload_buf, 0, payload_buf_len);
		serialize_short(common_len_serialize_buf, common_output_buf_len);
		memcpy(payload_buf, common_len_serialize_buf, 2);
		memcpy(payload_buf + 2, common_output_buf, common_output_buf_len);
		memcpy(payload_buf + 2 + common_output_buf_len, pubmsg->msg_body, pubmsg->msg_body_len);

		sdk_error = aes_cbc_128_enc_padding(sdk_kernel->session_key,
											payload_buf, common_output_buf_len + 2 + pubmsg->msg_body_len, payload_buf_len,
											payload_buf_enc, &enc_output_buf_len);
		if (sdk_error != mkernel_internal_succ)
		{
			break;
		}

		mqtt_msg.payload = (void *)payload_buf_enc;
		mqtt_msg.payloadlen = enc_output_buf_len;

		mqtt_result_code = MQTTPublish(&g_DasClient, publish_topic, &mqtt_msg);
		//ezdev_sdk_kernel_log_info(mqtt_result_code, mqtt_result_code, "MQTTPublish seq:%d", pubmsg->msg_seq);
		if (mqtt_result_code == MQTTPACKET_BUFFER_TOO_SHORT)
		{
			ezdev_sdk_kernel_log_error(mkernel_internal_call_mqtt_buffer_too_short, mqtt_result_code, "mqtt buffer too short\n");
			sdk_error = mkernel_internal_call_mqtt_buffer_too_short;
			break;
		}

		if (mqtt_result_code != 0)
		{
			ezdev_sdk_kernel_log_error(mkernel_internal_call_mqtt_pub_error, mqtt_result_code, "mqtt publish error\n");
			sdk_error = mkernel_internal_call_mqtt_pub_error;
			break;
		}

	} while (0);

	if(NULL != common_output_buf)
	{
		free(common_output_buf);
		common_output_buf = NULL;
	}
	if(NULL != payload_buf)
	{
		free(payload_buf);
		payload_buf = NULL;
	}
	if(NULL != payload_buf_enc)
	{
		free(payload_buf_enc);
		payload_buf_enc = NULL;
	}
	return sdk_error;
}



static mkernel_internal_error send_message_to_das_v3(ezdev_sdk_kernel *sdk_kernel)
{
	ezdev_sdk_kernel_pubmsg_exchange_v3 *ptr_pubmsg_exchange = NULL;
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	sdk_send_msg_ack_context_v3 context = {0};
	do
	{
		if (mkernel_internal_succ != (sdk_error = pop_queue_pubmsg_exchange_v3(&ptr_pubmsg_exchange)) ||
			NULL == ptr_pubmsg_exchange)
		{
			break;
		}
		
		sdk_error = das_send_pubmsg_v3(sdk_kernel, &ptr_pubmsg_exchange->msg_conntext_v3);
		
		ezdev_sdk_kernel_log_info(sdk_error, sdk_error, "pub msg v3 result, module:%s, resource_id:%s, resource_type:%s, msg_type:%s,ext_msg:%s seq:%d \n",
								  ptr_pubmsg_exchange->msg_conntext_v3.module, ptr_pubmsg_exchange->msg_conntext_v3.resource_id,\
								  ptr_pubmsg_exchange->msg_conntext_v3.resource_type,ptr_pubmsg_exchange->msg_conntext_v3.msg_type,\
								  ptr_pubmsg_exchange->msg_conntext_v3.ext_msg, ptr_pubmsg_exchange->msg_conntext_v3.msg_seq);

		if (mkernel_internal_call_mqtt_pub_error == sdk_error)
		{
			//发布失败，重连设备，并缓存指令
			if (ptr_pubmsg_exchange->max_send_count-- > 1)
			{
				ezdev_sdk_kernel_log_info(sdk_error, sdk_error, "v3 msg send failed,das need reconnect, count--:%d", ptr_pubmsg_exchange->max_send_count);
				push_queue_head_pubmsg_exchange_v3(ptr_pubmsg_exchange);
				return mkernel_internal_das_need_reconnect;
			}
		}
		ezdev_sdk_kernel_log_debug(sdk_error, 0, "broadcast_runtime_err, TAG_MSG_ACK_v3");
		context.msg_seq = ptr_pubmsg_exchange->msg_conntext_v3.msg_seq;
		context.msg_qos = ptr_pubmsg_exchange->msg_conntext_v3.msg_qos;

        strncpy(context.module, ptr_pubmsg_exchange->msg_conntext_v3.module, ezdev_sdk_module_name_len-1);
        strncpy(context.resource_id,ptr_pubmsg_exchange->msg_conntext_v3.resource_id,ezdev_sdk_resource_id_len-1);
		strncpy(context.resource_type, ptr_pubmsg_exchange->msg_conntext_v3.resource_type, ezdev_sdk_resource_type_len-1);
		strncpy(context.method, ptr_pubmsg_exchange->msg_conntext_v3.method, ezdev_sdk_method_len -1);
		strncpy(context.sub_serial, ptr_pubmsg_exchange->msg_conntext_v3.sub_serial, ezdev_sdk_max_serial_len-1);
		strncpy(context.msg_type, ptr_pubmsg_exchange->msg_conntext_v3.msg_type, ezdev_sdk_msg_type_len - 1);
		strncpy(context.ext_msg, ptr_pubmsg_exchange->msg_conntext_v3.ext_msg, ezdev_sdk_ext_msg_len - 1);

		if (mkernel_internal_succ != broadcast_runtime_err(TAG_MSG_ACK_V3, mkiE2ezE(sdk_error), &context, sizeof(context)))
		{
            ezdev_sdk_kernel_log_error(sdk_error, 0, "broadcast_runtime_err failed,module:%s ,msg_type:%s\n",context.module, context.msg_type);
		}
		if (ptr_pubmsg_exchange->msg_conntext_v3.msg_body)
			free(ptr_pubmsg_exchange->msg_conntext_v3.msg_body);

		free(ptr_pubmsg_exchange);
	} while (0);

	return sdk_error;	
}

static mkernel_internal_error send_message_to_das_v2(ezdev_sdk_kernel *sdk_kernel)
{
	char cRiskResult = 0;
	ezdev_sdk_kernel_pubmsg_exchange *ptr_pubmsg_exchange = NULL;
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	sdk_send_msg_ack_context context = {0};
	do
	{
		if (mkernel_internal_succ != (sdk_error = pop_queue_pubmsg_exchange(&ptr_pubmsg_exchange)) ||
			NULL == ptr_pubmsg_exchange)
		{
			break;
		}

		if (0 == (cRiskResult = check_cmd_risk_control(sdk_kernel, ptr_pubmsg_exchange->msg_conntext.msg_domain_id, ptr_pubmsg_exchange->msg_conntext.msg_command_id)))
		{
			sdk_error = das_send_pubmsg(sdk_kernel, &ptr_pubmsg_exchange->msg_conntext);
		}

		if (1 == cRiskResult)
			sdk_error = mkernel_internal_extend_no_find;
		else if (2 == cRiskResult)
			sdk_error = mkernel_internal_force_domain_risk;
		else if (3 == cRiskResult)
			sdk_error = mkernel_internal_force_cmd_risk;

		ezdev_sdk_kernel_log_info(sdk_error, sdk_error, "pub msg result, domain:%d ,cmd:%d, len:%d, seq:%d, qos:%d\n",
								  ptr_pubmsg_exchange->msg_conntext.msg_domain_id, ptr_pubmsg_exchange->msg_conntext.msg_command_id, ptr_pubmsg_exchange->msg_conntext.msg_body_len, ptr_pubmsg_exchange->msg_conntext.msg_seq, ptr_pubmsg_exchange->msg_conntext.msg_qos);

		if (mkernel_internal_call_mqtt_pub_error == sdk_error)
		{
			//发布失败，重连设备，并缓存指令
			if (ptr_pubmsg_exchange->max_send_count-- > 1)
			{
				ezdev_sdk_kernel_log_info(sdk_error, sdk_error, "ptr_pubmsg_exchange send failed,das need reconnect, count--:%d\n", ptr_pubmsg_exchange->max_send_count);
				push_queue_head_pubmsg_exchange(ptr_pubmsg_exchange);
				return mkernel_internal_das_need_reconnect;
			}
		}
		ezdev_sdk_kernel_log_debug(sdk_error, 0, "broadcast_runtime_err, TAG_MSG_ACK\n");
		context.msg_domain_id = ptr_pubmsg_exchange->msg_conntext.msg_domain_id;
		context.msg_command_id = ptr_pubmsg_exchange->msg_conntext.msg_command_id;
		context.msg_seq = ptr_pubmsg_exchange->msg_conntext.msg_seq;
		context.msg_qos = ptr_pubmsg_exchange->msg_conntext.msg_qos;
		context.externel_ctx = ptr_pubmsg_exchange->msg_conntext.externel_ctx;
		context.externel_ctx_len = ptr_pubmsg_exchange->msg_conntext.externel_ctx_len;

		if (mkernel_internal_succ != broadcast_runtime_err(TAG_MSG_ACK, mkiE2ezE(sdk_error), &context, sizeof(context)))
		{
			if (NULL == ptr_pubmsg_exchange->msg_conntext.externel_ctx)
				free(ptr_pubmsg_exchange->msg_conntext.externel_ctx);
		}

		if (ptr_pubmsg_exchange->msg_conntext.msg_body)
			free(ptr_pubmsg_exchange->msg_conntext.msg_body);

		free(ptr_pubmsg_exchange);
	} while (0);

	return sdk_error;

}


static mkernel_internal_error das_message_send(ezdev_sdk_kernel *sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	do
	{
		sdk_error = send_message_to_das_v2(sdk_kernel);
		if(mkernel_internal_succ != sdk_error&& mkernel_internal_queue_empty !=sdk_error)
		{
			break;
		}
		sdk_error = send_message_to_das_v3(sdk_kernel);

	}while(0);

	return sdk_error;
}

static mkernel_internal_error das_mqttlogin2das(ezdev_sdk_kernel *sdk_kernel, EZDEV_SDK_BOOL light_reg)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	EZDEV_SDK_INT32 mqtt_result_code = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	char sub_topic[128];
	unsigned char *will_message = NULL;
	EZDEV_SDK_UINT32 will_message_len = 0;

	memset(g_sendbuf, 0, ezdev_sdk_send_buf_max);
	memset(g_readbuf, 0, ezdev_sdk_recv_buf_max);

	do
	{
		sdk_error = MQTTNetConnect(&g_DasNetWork, sdk_kernel->redirect_das_info.das_address, sdk_kernel->redirect_das_info.das_port);
		if (sdk_error != mkernel_internal_succ)
		{
			ezdev_sdk_kernel_log_debug(sdk_error, sdk_error, "das_mqtt_reg2das NetworkConnect :%s :%d, error:%d\n",
									   sdk_kernel->redirect_das_info.das_address, sdk_kernel->redirect_das_info.das_port, sdk_error);
			break;
		}
		connectData.MQTTVersion = 4;
		if (light_reg)
		{
			connectData.cleansession = 0;
			connectData.willFlag = 1;
		}
		else
		{
			/**
			* \brief	0：表示设备断线重连
			1：表示设备重新上线
			*/
			connectData.cleansession = 1;
			/**
			* \brief	0:断线重连上线
			1:设备重新上线，包含遗嘱消息（设备信息）
			*/
			connectData.willFlag = 1;
		}
		/**
		* \brief		QoS0： 00	QoS1： 01	QoS2： 10	使用QoS1
		*/
		connectData.will.qos = 1;
		connectData.will.retained = 1;

		memset(&connectData.username, 0, sizeof(connectData.username));
		memset(&connectData.password, 0, sizeof(connectData.password));
		connectData.keepAliveInterval = sdk_kernel->das_keepalive_interval;
		if (strcmp("", (const char*)sdk_kernel->dev_id) == 0)
		{
			ezdev_sdk_kernel_log_error(0, 0, "das_mqttlogin2das,dev_id is empty!!!\n");
			sdk_error = mkernel_internal_input_param_invalid;
			break;
		}
		connectData.clientID.lenstring.data = "";
		connectData.clientID.lenstring.len = 0;
		connectData.username.cstring = sdk_kernel->dev_info.dev_subserial;
		connectData.password.cstring = "test";
		memset(sub_topic, 0, 128);

		if(g_is_first_session)
			snprintf(sub_topic, 128, "/Basic/pu2cenplt/%s/firstconnect", sdk_kernel->dev_info.dev_subserial);
		else
			snprintf(sub_topic, 128, "/Basic/pu2cenplt/%s/breakconnect", sdk_kernel->dev_info.dev_subserial);

		connectData.will.topicName.lenstring.data = sub_topic;
		connectData.will.topicName.lenstring.len = 128;

		if (!light_reg)
		{
			sdk_error = serialize_devinfo(sdk_kernel, &will_message, &will_message_len);
			if (sdk_error != mkernel_internal_succ)
			{
				ezdev_sdk_kernel_log_error(sdk_error, mqtt_result_code, "mqtt serialize_devinfo error\n");
				break;
			}
			connectData.will.message.lenstring.data = (char *)will_message;
			connectData.will.message.lenstring.len = will_message_len;
		}
		else
		{
			sdk_error = serialize_lightreginfo(sdk_kernel, &will_message, &will_message_len);
			if (sdk_error != mkernel_internal_succ)
			{
				ezdev_sdk_kernel_log_error(sdk_error, mqtt_result_code, "serialize_lightreginfo error\n");
				break;
			}
			connectData.will.message.lenstring.data = (char *)will_message;
			connectData.will.message.lenstring.len = will_message_len;
		}

		if (0 != (mqtt_result_code = MQTTConnect(&g_DasClient, &connectData)))
		{
			if (FAILURE == mqtt_result_code)
				sdk_error = mkernel_internal_call_mqtt_connect;
			else
			{
				sdk_error = mkernel_internal_mqtt_error_begin + mqtt_result_code;
			    ezdev_sdk_kernel_log_error(sdk_error, mqtt_result_code, "mqtt connect error\n");
			}
				
			break;
		}

		g_is_first_session = EZDEV_SDK_FALSE;
	} while (0);

	if (will_message != NULL)
	{
		free(will_message);
		will_message = NULL;
	}

	if (sdk_error != mkernel_internal_succ)
	{
		MQTTNetDisconnect(&g_DasNetWork);
		MQTTNetFini(&g_DasNetWork);
	}
	ezdev_sdk_kernel_log_error(sdk_error, sdk_error, "mqtt connect server, server ip:%s, port:%d\n", sdk_kernel->redirect_das_info.das_address, sdk_kernel->redirect_das_info.das_port);
	return sdk_error;
}

static mkernel_internal_error das_mqtt_logout2das()
{
	EZDEV_SDK_INT32 mqtt_code = 0;
	mqtt_code = MQTTDisconnect(&g_DasClient);
	if (mqtt_code != 0)
	{
		ezdev_sdk_kernel_log_warn(mkernel_internal_call_mqtt_disconnect, mqtt_code, "das_mqtt_logout2das error:%d\n", mqtt_code);
	}
	MQTTNetDisconnect(&g_DasNetWork);
	MQTTNetFini(&g_DasNetWork);
	ezdev_sdk_kernel_log_debug(0, 0, "das_mqtt_logout2das return \n");
	return mkernel_internal_succ;
}

void das_object_init(ezdev_sdk_kernel *sdk_kernel)
{
	EZDEV_SDK_UNUSED(sdk_kernel)
	MQTTNetInit(&g_DasNetWork);

	//	MQTTClientInit(&g_DasClient, &g_DasNetWork, 10*1000, g_sendbuf, ezdev_sdk_send_buf_max, g_readbuf, ezdev_sdk_recv_buf_max);

	memset(g_sendbuf, 0, ezdev_sdk_send_buf_max);
	memset(g_readbuf, 0, ezdev_sdk_recv_buf_max);

	MQTTClientInit(&g_DasClient, &g_DasNetWork, 6 * 1000, g_sendbuf, ezdev_sdk_send_buf_max, g_readbuf, ezdev_sdk_recv_buf_max);
    
	/*if(das_getGoTcpAlways() == 0)
	{
        coapClientInit(sdk_kernel, &g_DasClientByCoap);
        g_DasClientByCoap.sendData2Up = sendData2Up4coap;
        g_DasClientByCoap.genaralSeq = genaral_seq;
    }*/

	/* 初始化消息队列 */
	init_queue(ezdev_sdk_queue_max, ezdev_sdk_queue_max, ezdev_sdk_queue_max * 4);
}

void das_object_fini(ezdev_sdk_kernel *sdk_kernel)
{
	EZDEV_SDK_UNUSED(sdk_kernel)
	memset(g_sendbuf, 0, ezdev_sdk_send_buf_max);
	memset(g_readbuf, 0, ezdev_sdk_recv_buf_max);

	MQTTClientFini(&g_DasClient);

	MQTTNetFini(&g_DasNetWork);
	fini_queue();

	g_das_transport_seq = 0;
}

mkernel_internal_error das_reg(ezdev_sdk_kernel *sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	sdk_error = das_mqttlogin2das(sdk_kernel, EZDEV_SDK_FALSE);
	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = das_subscribe_revc_topic(sdk_kernel, 1);
	if (sdk_error != mkernel_internal_succ)
	{
		das_mqtt_logout2das();
		ezdev_sdk_kernel_log_warn(sdk_error, 0, "das_reg subscribe error\n");
	}

	ezdev_sdk_kernel_log_debug(sdk_error, sdk_error, "das_reg result:%d\n", sdk_error);
	return sdk_error;
}

mkernel_internal_error das_light_reg(ezdev_sdk_kernel *sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	das_mqtt_logout2das();

	sdk_error = das_mqttlogin2das(sdk_kernel, EZDEV_SDK_TRUE);

	ezdev_sdk_kernel_log_debug(sdk_error, sdk_error, "das_light_reg result:%d\n", sdk_error);

	return sdk_error;
}

mkernel_internal_error das_light_reg_v2(ezdev_sdk_kernel *sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	sdk_error = das_mqttlogin2das(sdk_kernel, EZDEV_SDK_TRUE);

	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = das_subscribe_revc_topic(sdk_kernel, 1);
	if (sdk_error != mkernel_internal_succ)
	{
		das_mqtt_logout2das();
		ezdev_sdk_kernel_log_warn(sdk_error, 0, "das_light_reg_v2 subscribe error\n");
	}

	ezdev_sdk_kernel_log_debug(sdk_error, sdk_error, "das_light_reg result:%d\n", sdk_error);

	return sdk_error;
}
/** 
*  \brief		RF快速重连
*  \param[in] 	ezdev_sdk_kernel * sdk_kernel
*  \return 		mkernel_internal_error
*/
mkernel_internal_error das_light_reg_v3(ezdev_sdk_kernel *sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	sdk_error = das_mqttlogin2das(sdk_kernel, EZDEV_SDK_FALSE);

	if (sdk_error != mkernel_internal_succ)
	{
		return sdk_error;
	}

	sdk_error = das_subscribe_revc_topic(sdk_kernel, 1);
	if (sdk_error != mkernel_internal_succ)
	{
		das_mqtt_logout2das();
		ezdev_sdk_kernel_log_warn(sdk_error, 0, "das_light_reg_v3 subscribe error\n");
	}

	return sdk_error;
}

mkernel_internal_error das_unreg(ezdev_sdk_kernel *sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	ezdev_sdk_kernel_log_debug(0, 0, "das_unreg close revc_topic \n");
	sdk_error = das_subscribe_revc_topic(sdk_kernel, 0);
	if (sdk_error != mkernel_internal_succ)
	{
		ezdev_sdk_kernel_log_warn(sdk_error, 0, "das_unreg unsubscribe error\n");
	}
	das_mqtt_logout2das();
	ezdev_sdk_kernel_log_info(0, 0, "das_unreg complete");
	return mkernel_internal_succ;
}

/** 
*  \brief		非阻塞发布消息
*  \method		das_send_pubmsg_async
*  \param[in] 	ezdev_sdk_kernel * sdk_kernel
*  \param[in] 	const ezdev_sdk_kernel_pubmsg * msg
*  \return 		mkernel_internal_error
*/
mkernel_internal_error das_send_pubmsg_async(ezdev_sdk_kernel *sdk_kernel, ezdev_sdk_kernel_pubmsg_exchange *msg_exchange)
{
	/**
	* \brief   将消息放到发布队列中去
	*/
	mkernel_internal_error sdk_error = push_queue_pubmsg_exchange(msg_exchange);
	EZDEV_SDK_UNUSED(sdk_kernel);
	return sdk_error;
}

/** 
*  \brief		非阻塞发布消息
*  \method		das_send_pubmsg_async_v3
*  \param[in] 	ezdev_sdk_kernel * sdk_kernel
*  \param[in] 	const ezdev_sdk_kernel_pubmsg_v3 * msg
*  \return 		mkernel_internal_error
*/
mkernel_internal_error das_send_pubmsg_async_v3(ezdev_sdk_kernel *sdk_kernel, ezdev_sdk_kernel_pubmsg_exchange_v3 *msg_exchange)
{
	/**
	* \brief   将消息放到发布队列中去
	*/
	mkernel_internal_error sdk_error = push_queue_pubmsg_exchange_v3(msg_exchange);
	EZDEV_SDK_UNUSED(sdk_kernel);
	return sdk_error;
}

mkernel_internal_error das_change_keep_alive_interval(ezdev_sdk_kernel *sdk_kernel, EZDEV_SDK_UINT16 interval)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	EZDEV_SDK_UNUSED(sdk_kernel)
    ezdev_sdk_kernel_log_info(0, 0, "das_change_keep_alive interval:%d \n",interval);
	sdk_kernel->das_keepalive_interval = interval;
	g_DasClient.keepAliveInterval = interval;
	TimerCountdown(&g_DasClient.ping_timer, interval);

	return sdk_error;
}

static mkernel_internal_error das_subscribe_revc_topic(ezdev_sdk_kernel *sdk_kernel, EZDEV_SDK_INT8 open)
{
	/**
	* \brief   订阅topic 来接收数据 /设备序列号/#
	*/
	EZDEV_SDK_INT32 mqtt_result_code = 0;
	char subscribe_topic[ezdev_sdk_recv_topic_len];
    memset(subscribe_topic, 0, ezdev_sdk_recv_topic_len);
	if (open)
	{
		if(sdk_v3_reged == sdk_kernel->v3_reg_status)
		{
			snprintf(subscribe_topic, ezdev_sdk_recv_topic_len, "/iot/%s/#", sdk_kernel->dev_info.dev_subserial);
			ezdev_sdk_kernel_log_debug(0, 0, "mqtt subscribe:%s ", subscribe_topic);
			mqtt_result_code = MQTTSubscribe(&g_DasClient, subscribe_topic, QOS1, das_message_receive_v3);
			if (mqtt_result_code != 0)
			{
				ezdev_sdk_kernel_log_error(mkernel_internal_call_mqtt_sub_error, mqtt_result_code, "mqtt subscribe:%s error\n", subscribe_topic);
				return mkernel_internal_call_mqtt_sub_error;
			}
		}
		
        memset(subscribe_topic, 0,ezdev_sdk_recv_topic_len);
		snprintf(subscribe_topic, ezdev_sdk_recv_topic_len, "/%s/#", sdk_kernel->dev_info.dev_subserial);
		mqtt_result_code = MQTTSubscribe(&g_DasClient, subscribe_topic, QOS1, das_message_receive);
		if (mqtt_result_code != 0)
		{
			ezdev_sdk_kernel_log_error(mkernel_internal_call_mqtt_sub_error, mqtt_result_code, "mqtt subscribe:%s error\n", subscribe_topic);
			return mkernel_internal_call_mqtt_sub_error;
		}
		ezdev_sdk_kernel_log_debug(0, 0, "mqtt subscribe old topic:%s \n", subscribe_topic);
	}
	else
	{
		if(sdk_v3_reged == sdk_kernel->v3_reg_status)
		{
			snprintf(subscribe_topic, ezdev_sdk_recv_topic_len, "/iot/%s/#", sdk_kernel->dev_info.dev_subserial);
			ezdev_sdk_kernel_log_debug(0, 0, "mqtt subscribe:%s \n", subscribe_topic);
			mqtt_result_code = MQTTUnsubscribe(&g_DasClient, subscribe_topic);
			if (mqtt_result_code != 0)
			{
				ezdev_sdk_kernel_log_warn(mkernel_internal_call_mqtt_sub_error, mqtt_result_code, "mqtt unsubscribe:%s error\n", subscribe_topic);
		    }
		}
		memset(subscribe_topic, 0,ezdev_sdk_recv_topic_len);
		snprintf(subscribe_topic, ezdev_sdk_recv_topic_len, "/%s/#", sdk_kernel->dev_info.dev_subserial);
		mqtt_result_code = MQTTUnsubscribe(&g_DasClient, subscribe_topic);
		if (mqtt_result_code != 0)
		{
			ezdev_sdk_kernel_log_warn(mkernel_internal_call_mqtt_sub_error, mqtt_result_code, "mqtt unsubscribe:%s error\n", subscribe_topic);
		}
	}
	return mkernel_internal_succ;
}

mkernel_internal_error das_yield(ezdev_sdk_kernel *sdk_kernel)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;

	int mqtt_result = MQTTYield(&g_DasClient, 10);
	if (mqtt_result != 0)
	{
		ezdev_sdk_kernel_log_debug(mkernel_internal_call_mqtt_yield_error, mqtt_result, "das_yield MQTTYield:%d error\n", mqtt_result);
	}

	do
	{
		/**
		* \brief   判断错误，看socket是否有异常
		*/
		if (MQTTNetGetLastError() == mkernel_internal_net_socket_error || MQTTNetGetLastError() == mkernel_internal_net_socket_closed)
		{
			sdk_error = mkernel_internal_das_need_reconnect;
			ezdev_sdk_kernel_log_error(sdk_error, sdk_error, "socket error need rereg\n");
			break;
		}
		/**
		* \brief   更新超时时间
		*/
		if (g_DasClient.keepAliveInterval != sdk_kernel->das_keepalive_interval)
		{
			g_DasClient.keepAliveInterval = sdk_kernel->das_keepalive_interval;
		}
		/**
		* \brief   判断心跳是否超时
		*/
		if (!TimerIsExpiredByDiff(&g_DasClient.connect_timer, g_DasClient.keepAliveInterval * 2000))
		{
			//信令发送失败已经在内部上抛

			if (mkernel_internal_das_need_reconnect != (sdk_error = das_message_send(sdk_kernel)))
				sdk_error = mkernel_internal_succ;
		}
		else
		{
			sdk_error = mkernel_internal_das_need_reconnect;
			ezdev_sdk_kernel_log_error(sdk_error, sdk_error, "heart timeout need rereg\n");
		}
	} while (0);

	return sdk_error;
}

int ezdev_sdk_kernel_get_das_socket(ezdev_sdk_kernel *sdk_kernel)
{
	return sdk_kernel->platform_handle.net_work_getsocket(g_DasNetWork.my_socket);
}


/*void das_checkCoapInfoFile(ezdev_sdk_kernel* sdk_kernel)
{
    unsigned char loadCfgValue[COAP_SAVE_CFG_FILE_MAX_LEN];
    memset(&loadCfgValue, 0, sizeof(loadCfgValue));

    sdk_kernel->platform_handle.key_value_load(sdk_keyvalue_coapinfo, loadCfgValue, COAP_SAVE_CFG_FILE_MAX_LEN);
    ezdev_sdk_kernel_log_error(0, 0,"[check]%s\n", loadCfgValue);
    if((strlen(loadCfgValue) == 0) || (coapLoad_cfgInfo(&sdk_kernel->coapInfo) != OK))
	{
        ezdev_sdk_kernel_log_error(0, 0, "[das_checkCoapInfoFile] reset coapInfo file.\n");
        sdk_kernel->coapInfo.protocol = EZDEV_SDK_PROTOCOL_COAP;
        sdk_kernel->coapInfo.isCheckUdpPortAging = true;
        sdk_kernel->coapInfo.keepAliveInterval = 0;
        sdk_kernel->coapInfo.lastAliveTime = 0;
        coapSave_cfgInfo(&sdk_kernel->coapInfo);
    }
    return;
}*/
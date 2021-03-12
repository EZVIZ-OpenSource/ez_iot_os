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

#include "ezdev_sdk_kernel_extend.h"
#include "sdk_kernel_def.h"
#include "ezdev_sdk_kernel_struct.h"
#include "ezdev_sdk_kernel_error.h"
#include "mkernel_internal_error.h"
#include "access_domain_bus.h"
#include "dev_protocol_def.h"
#include "ezdev_sdk_kerne_queuel.h"

#include "bscJSON.h"

EXTERN_QUEUE_FUN(submsg)
EXTERN_QUEUE_FUN(inner_cb_notic)
EXTERN_QUEUE_FUN(pubmsg_exchange)

EXTERN_QUEUE_FUN(submsg_v3)
EXTERN_QUEUE_FUN(pubmsg_exchange_v3)
EXTERN_QUEUE_BASE_FUN


static EZDEV_SDK_UINT16 g_kernel_domains_count = 0;                             ///<	扩展数
static EZDEV_SDK_UINT16 g_kernel_extend_count = 0;                              ///<	扩展数
static ezdev_sdk_kernel_domain_info g_kernel_domains[ezdev_sdk_extend_count];   ///<	扩展列表
static ezdev_sdk_kernel_domain_info_v3 g_kernel_extend[ezdev_sdk_extend_count]; ///<	扩展列表 V3协议
static sdk_kernel_event_notice g_kernel_event_notice_cb;                        ///<	SDK回调给上层的通知消息

void extend_init(sdk_kernel_event_notice kernel_event_notice_cb)
{
    g_kernel_domains_count = ezdev_sdk_extend_count;
    g_kernel_extend_count = ezdev_sdk_extend_count;
    memset(&g_kernel_domains, 0, sizeof(ezdev_sdk_kernel_domain_info) * g_kernel_domains_count);
    memset(&g_kernel_extend, 0, sizeof(ezdev_sdk_kernel_domain_info_v3) * g_kernel_extend_count);
    g_kernel_event_notice_cb = kernel_event_notice_cb;
}

void extend_fini()
{
    g_kernel_event_notice_cb = NULL;
    memset(&g_kernel_domains, 0, sizeof(ezdev_sdk_kernel_domain_info) * g_kernel_domains_count);
    memset(&g_kernel_extend, 0, sizeof(ezdev_sdk_kernel_domain_info_v3) * g_kernel_extend_count);
    g_kernel_domains_count = 0;
    g_kernel_extend_count = 0;
}

/** 
 *  \brief		根据领域ID找到领域上下文
 *  \method		extend_get
 *  \param[in] 	domain_id	领域ID
 *  \return		成功返回领域上下文指针 失败返回NULL
 */
ezdev_sdk_kernel_domain_info *extend_get(EZDEV_SDK_UINT32 domain_id)
{
    EZDEV_SDK_UINT16 index = 0;
    for (index = 0; index < g_kernel_domains_count; index++)
    {
        if (g_kernel_domains[index].kernel_extend.domain_id == domain_id)
        {
            return &g_kernel_domains[index];
        }
    }
    return NULL;
}

/** 
 *  \brief		根据extend_id找到领域上下文
 *  \method		extend_get
 *  \param[in] 	domain_id	领域ID
 *  \return		成功返回领域上下文指针 失败返回NULL
 */
ezdev_sdk_kernel_domain_info_v3 *extend_get_by_extend_id(const char* module)
{
    EZDEV_SDK_UINT16 index = 0;
    if(NULL == module)
    {
        ezdev_sdk_kernel_log_error(mkernel_internal_extend_id_error, 0, "extend_get input err\n");
        return NULL;
    }
    for (index = 0; index < g_kernel_extend_count; index++)
    {
        if (0 == strcmp(g_kernel_extend[index].kernel_extend.module, module))
        {
            return &g_kernel_extend[index];
        }
    }
    return NULL;
}

/** 
 *  \brief		服务器消息分发
 *  \method		consume_extend_data
 *	\note		将来自服务器的消息队列中按先进先出的原则获取第一个消息，
 *				根据消息的领域ID找到注册的领域，调用领域的回调函数，将消息分发到该领域
 *  \param[in] 	sdk_kernel				微内核上下文
 *  \return		成功返回0 失败详见错误码
 */

static mkernel_internal_error consume_extend_data_v3(ezdev_sdk_kernel *sdk_kernel)
{
    /**
	* \brief   用来驱动扩展的消息发送
	*/
    const ezdev_sdk_kernel_domain_info_v3 *kernel_extend = NULL;
    ezdev_sdk_kernel_submsg_v3 *ptr_submsg = NULL;
    mkernel_internal_error kernel_error = mkernel_internal_succ;
    do
    {
        kernel_error = pop_queue_submsg_v3(&ptr_submsg);
        if(kernel_error == mkernel_internal_queue_empty)
        {
            break;
        }
        if(kernel_error != mkernel_internal_succ || ptr_submsg == NULL)
        {
            ezdev_sdk_kernel_log_error(kernel_error, 0, "extend_yield pop_queue_submsg v3 error");
            break;
        }
        if(strlen(ptr_submsg->module) > 0)
        {
            kernel_extend = extend_get_by_extend_id(ptr_submsg->module);
            if (kernel_extend == NULL)
            {
                kernel_error = mkernel_internal_extend_no_find;
                ezdev_sdk_kernel_log_error(kernel_error, 0, "no find module %s , seq:%d \n", ptr_submsg->module, ptr_submsg->msg_seq);
            }
            else if(kernel_extend->kernel_extend.ezdev_sdk_kernel_data_route)
            {
                ezdev_sdk_kernel_log_debug(0, 0, "sdk_data_route v3, module:%s,msg_type:%s ,seq:%d\n", ptr_submsg->module, ptr_submsg->msg_type, ptr_submsg->msg_seq);
                kernel_extend->kernel_extend.ezdev_sdk_kernel_data_route(ptr_submsg);
            }
        }
        if(ptr_submsg != NULL)
        {
            ezdev_sdk_kernel_log_trace(kernel_error, 0, "rev msg v3 module:%s, resource_id:%s, resource_type:%s, msg_type:%s, seq:%d, len:%d", ptr_submsg->module, \
                                      ptr_submsg->resource_id, ptr_submsg->resource_type,  ptr_submsg->msg_type, ptr_submsg->msg_seq, ptr_submsg->buf_len);

            if (ptr_submsg->buf != NULL)
            {
                free(ptr_submsg->buf);
                ptr_submsg->buf = NULL;
            }
            free(ptr_submsg);
            ptr_submsg = NULL;
        }

    } while (1);

    return kernel_error;
}

/** 
 *  \brief		服务器消息分发 2.0 协议
 *  \method		consume_extend_data
 *	\note		将来自服务器的消息队列中按先进先出的原则获取第一个消息，
 *				根据消息的领域ID找到注册的领域，调用领域的回调函数，将消息分发到该领域
 *  \param[in] 	sdk_kernel				微内核上下文
 *  \return		成功返回0 失败详见错误码
 */
static mkernel_internal_error consume_extend_data(ezdev_sdk_kernel *sdk_kernel)
{
    /**
	* \brief   用来驱动扩展的消息发送
	*/
    const ezdev_sdk_kernel_domain_info *kernel_domain = NULL;
    ezdev_sdk_kernel_submsg *ptr_submsg = NULL;
    mkernel_internal_error kernel_error = mkernel_internal_succ;

    do
    {
        kernel_error = pop_queue_submsg(&ptr_submsg);

        if (kernel_error == mkernel_internal_queue_empty)
        {
            break;
        }
        if (kernel_error != mkernel_internal_succ || ptr_submsg == NULL)
        {
            ezdev_sdk_kernel_log_debug(kernel_error, 0, "extend_yield pop_queue_submsg error");
            break;
        }

        kernel_domain = extend_get(ptr_submsg->msg_domain_id);
        if (kernel_domain == NULL)
        {
            kernel_error = mkernel_internal_extend_no_find;
            ezdev_sdk_kernel_log_debug(kernel_error, 0, "no find domain:%d cmd:%d \n", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id);
        }
        else
        {
            ezdev_sdk_kernel_log_debug(0, 0, "data_routing:domain:%d cmd:%d, seq:%d\n", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id, ptr_submsg->msg_seq);
            kernel_domain->kernel_extend.ezdev_sdk_kernel_extend_data_route(ptr_submsg, kernel_domain->kernel_extend.pUser);
        }

        if (ptr_submsg != NULL)
        {
            ezdev_sdk_kernel_log_trace(kernel_error, 0, "rev msg domain:%d cmd:%d, seq:%d, len:%d", ptr_submsg->msg_domain_id, ptr_submsg->msg_command_id, ptr_submsg->msg_seq, ptr_submsg->buf_len);

            if (ptr_submsg->buf != NULL)
            {
                free(ptr_submsg->buf);
                ptr_submsg->buf = NULL;
            }

            free(ptr_submsg);
            ptr_submsg = NULL;
        }

    } while (1);

    return kernel_error;
}

/** 
 *  \brief		本地消息分发
 *  \method		consume_extend_event
 *	\note		将来自本地各线程的消息队列中按先进先出的原则获取第一个消息，
 *				将该消息分发至所有领域，如果是extend_cb_event类事件，还将分发到上层应用
 *  \return		成功返回0 失败详见错误码
 */
/*static mkernel_internal_error consume_extend_event_v3()
{
    EZDEV_SDK_UINT16 index = 0;
    mkernel_internal_error kernel_error = mkernel_internal_succ;
    ezdev_sdk_kernel_inner_cb_notic *ptr_inner_cb_notic = NULL;
    kernel_error = pop_queue_inner_cb_notic(&ptr_inner_cb_notic);
    if (kernel_error == mkernel_internal_queue_empty)
    {
        return kernel_error;
    }
    if (kernel_error != mkernel_internal_succ || NULL == ptr_inner_cb_notic)
    {
        ezdev_sdk_kernel_log_debug(kernel_error, 0, "pop_queue_event error");
        return kernel_error;
    }

    if (ptr_inner_cb_notic->cb_type == extend_cb_start)
    {
        for (index = 0; index < ezdev_sdk_extend_count; index++)
        {
            if (g_kernel_extend[index].kernel_extend.extend_id == 0 || g_kernel_extend[index].kernel_extend.ezdev_sdk_kernel_extend_start == NULL)
                break;
            g_kernel_extend[index].kernel_extend.ezdev_sdk_kernel_extend_start(g_kernel_extend[index].kernel_extend.pUser);
        }
    }
    else if (ptr_inner_cb_notic->cb_type == extend_cb_stop)
    {
        for (index = 0; index < ezdev_sdk_extend_count; index++)
        {
            if (g_kernel_extend[index].kernel_extend.extend_id == 0 || g_kernel_extend[index].kernel_extend.ezdev_sdk_kernel_extend_stop == NULL)
                break;
            g_kernel_extend[index].kernel_extend.ezdev_sdk_kernel_extend_stop(g_kernel_extend[index].kernel_extend.pUser);
        }
    }
    else if (ptr_inner_cb_notic->cb_type == extend_cb_event)
    {
        sdk_send_msg_ack_context *ptr_ack_ctx = NULL;
        if (sdk_kernel_event_runtime_err == ptr_inner_cb_notic->cb_event.event_type)
        {
            sdk_runtime_err_context *rt_err_ctx = (sdk_runtime_err_context *)(ptr_inner_cb_notic->cb_event.event_context);
            if (TAG_MSG_ACK == rt_err_ctx->err_tag)
                ptr_ack_ctx = (sdk_send_msg_ack_context *)rt_err_ctx->err_ctx;
        }

        for (index = 0; index < ezdev_sdk_extend_count; index++)
        {
            if (g_kernel_extend[index].kernel_extend.extend_id == 0 || g_kernel_extend[index].kernel_extend.ezdev_sdk_kernel_extend_event == NULL)
                break;

            //消息回执只回调给对应的领域
            if (ptr_ack_ctx && g_kernel_extend[index].kernel_extend.extend_id != ptr_ack_ctx->msg_extend_id)
                continue;

            g_kernel_extend[index].kernel_extend.ezdev_sdk_kernel_extend_event(&ptr_inner_cb_notic->cb_event, g_kernel_extend[index].kernel_extend.pUser);
        }

        //消息回执不需要回调给APP
        if (NULL == ptr_ack_ctx)
            g_kernel_event_notice_cb(&ptr_inner_cb_notic->cb_event);
    }

    destroy_inner_cb_notic(ptr_inner_cb_notic);

    return kernel_error;
}*/


/** 
 *  \brief		本地消息分发
 *  \method		consume_extend_event
 *	\note		将来自本地各线程的消息队列中按先进先出的原则获取第一个消息，
 *				将该消息分发至所有领域，如果是extend_cb_event类事件，还将分发到上层应用
 *  \return		成功返回0 失败详见错误码
 */
static mkernel_internal_error consume_extend_event()
{
    EZDEV_SDK_UINT16 index = 0;
    mkernel_internal_error kernel_error = mkernel_internal_succ;
    ezdev_sdk_kernel_inner_cb_notic *ptr_inner_cb_notic = NULL;
    kernel_error = pop_queue_inner_cb_notic(&ptr_inner_cb_notic);
    if (kernel_error == mkernel_internal_queue_empty)
    {
        return kernel_error;
    }
    if (kernel_error != mkernel_internal_succ || NULL == ptr_inner_cb_notic)
    {
        ezdev_sdk_kernel_log_debug(kernel_error, 0, "pop_queue_event error");
        return kernel_error;
    }

    if (ptr_inner_cb_notic->cb_type == extend_cb_start)
    {
        for (index = 0; index < ezdev_sdk_extend_count; index++)
        {
            if (g_kernel_domains[index].kernel_extend.domain_id == 0 || g_kernel_domains[index].kernel_extend.ezdev_sdk_kernel_extend_start == NULL)
                break;
            g_kernel_domains[index].kernel_extend.ezdev_sdk_kernel_extend_start(g_kernel_domains[index].kernel_extend.pUser);
        }
    }
    else if (ptr_inner_cb_notic->cb_type == extend_cb_stop)
    {
        for (index = 0; index < ezdev_sdk_extend_count; index++)
        {
            if (g_kernel_domains[index].kernel_extend.domain_id == 0 || g_kernel_domains[index].kernel_extend.ezdev_sdk_kernel_extend_stop == NULL)
                break;
            g_kernel_domains[index].kernel_extend.ezdev_sdk_kernel_extend_stop(g_kernel_domains[index].kernel_extend.pUser);
        }
    }
    else if (ptr_inner_cb_notic->cb_type == extend_cb_event)
    {
        sdk_send_msg_ack_context *ptr_ack_ctx = NULL;
        sdk_send_msg_ack_context_v3 *ptr_ack_ctx_v3 = NULL;
        if (sdk_kernel_event_runtime_err == ptr_inner_cb_notic->cb_event.event_type)
        {
            sdk_runtime_err_context *rt_err_ctx = (sdk_runtime_err_context *)(ptr_inner_cb_notic->cb_event.event_context);
            switch(rt_err_ctx->err_tag)
            {
                case TAG_MSG_ACK:
                    {
                        ptr_ack_ctx = (sdk_send_msg_ack_context *)rt_err_ctx->err_ctx;
                    }
                    break;
                case TAG_MSG_ACK_V3:
                    {
                        ptr_ack_ctx_v3 = (sdk_send_msg_ack_context_v3 *)rt_err_ctx->err_ctx;
                    }
                    break;  
                default:
                    break;   
            }  
        }
        for (index = 0; index < ezdev_sdk_extend_count; index++)
        {
            if(g_kernel_domains[index].kernel_extend.domain_id == 0 || g_kernel_domains[index].kernel_extend.ezdev_sdk_kernel_extend_event == NULL)
                break;
            //消息回执只回调给对应的领域
            if((ptr_ack_ctx && g_kernel_domains[index].kernel_extend.domain_id != ptr_ack_ctx->msg_domain_id)||ptr_ack_ctx_v3)
                continue;
            g_kernel_domains[index].kernel_extend.ezdev_sdk_kernel_extend_event(&ptr_inner_cb_notic->cb_event, g_kernel_domains[index].kernel_extend.pUser);
        }
        for (index = 0; index < ezdev_sdk_extend_count; index++)
        {
            if (strlen(g_kernel_extend[index].kernel_extend.module) == 0 || 
                g_kernel_extend[index].kernel_extend.ezdev_sdk_kernel_event_route == NULL)
            {
                break;
            }
            if ((ptr_ack_ctx_v3 && (0!=strcmp(g_kernel_extend[index].kernel_extend.module, ptr_ack_ctx_v3->module)))||ptr_ack_ctx)
                continue;
            g_kernel_extend[index].kernel_extend.ezdev_sdk_kernel_event_route(&ptr_inner_cb_notic->cb_event);
        }
        //消息回执不需要回调给APP
        if (NULL == ptr_ack_ctx && NULL == ptr_ack_ctx_v3)
        {
            g_kernel_event_notice_cb(&ptr_inner_cb_notic->cb_event);
        }
    }

    destroy_inner_cb_notic(ptr_inner_cb_notic);

    return kernel_error;
}

mkernel_internal_error clear_queue_pubmsg_exchange()
{
    ezdev_sdk_kernel_pubmsg_exchange *ptr_pubmsg_exchange = NULL;
    mkernel_internal_error kernel_error = mkernel_internal_succ;

    do
    {
        kernel_error = pop_queue_pubmsg_exchange(&ptr_pubmsg_exchange);
        if (kernel_error != mkernel_internal_succ || ptr_pubmsg_exchange == NULL)
        {
            break;
        }

        if (NULL != ptr_pubmsg_exchange->msg_conntext.msg_body)
            free(ptr_pubmsg_exchange->msg_conntext.msg_body);

        if (NULL != ptr_pubmsg_exchange->msg_conntext.externel_ctx)
            free(ptr_pubmsg_exchange->msg_conntext.externel_ctx);

        free(ptr_pubmsg_exchange);
        ptr_pubmsg_exchange = NULL;
    } while (1);

    return mkernel_internal_succ;
}

mkernel_internal_error clear_queue_pubmsg_exchange_v3()
{
    ezdev_sdk_kernel_pubmsg_exchange_v3 *ptr_pubmsg_exchange = NULL;
    mkernel_internal_error kernel_error = mkernel_internal_succ;

    do
    {
        kernel_error = pop_queue_pubmsg_exchange_v3(&ptr_pubmsg_exchange);
        if (kernel_error != mkernel_internal_succ || ptr_pubmsg_exchange == NULL)
        {
            break;
        }

        if (NULL != ptr_pubmsg_exchange->msg_conntext_v3.msg_body)
            free(ptr_pubmsg_exchange->msg_conntext_v3.msg_body);

        free(ptr_pubmsg_exchange);
        ptr_pubmsg_exchange = NULL;
    } while (1);

    return mkernel_internal_succ;
}

mkernel_internal_error clear_queue_submsg()
{
    ezdev_sdk_kernel_submsg *ptr_submsg = NULL;
    mkernel_internal_error kernel_error = mkernel_internal_succ;

    do
    {
        kernel_error = pop_queue_submsg(&ptr_submsg);
        if (kernel_error != mkernel_internal_succ || ptr_submsg == NULL)
        {
            break;
        }

        if (NULL != ptr_submsg->buf)
            free(ptr_submsg->buf);

        free(ptr_submsg);
        ptr_submsg = NULL;
    } while (1);

    return mkernel_internal_succ;
}


mkernel_internal_error clear_queue_submsg_v3()
{
    ezdev_sdk_kernel_submsg_v3 *ptr_submsg = NULL;
    mkernel_internal_error kernel_error = mkernel_internal_succ;

    do
    {
        kernel_error = pop_queue_submsg_v3(&ptr_submsg);
        if (kernel_error != mkernel_internal_succ || ptr_submsg == NULL)
        {
            break;
        }

        if (NULL != ptr_submsg->buf)
            free(ptr_submsg->buf);

        free(ptr_submsg);
        ptr_submsg = NULL;
    } while (1);

    return mkernel_internal_succ;
}

mkernel_internal_error extend_yield(ezdev_sdk_kernel *sdk_kernel)
{
    mkernel_internal_error mki_err = mkernel_internal_succ;

    if (sdk_stop == sdk_kernel->my_state)
    {
        // 如果sdk已关闭，需要将本地的所有消息全部上抛
        while (mkernel_internal_queue_empty != consume_extend_event())
        {
        };
        return mkernel_internal_haven_stop;
    }
    else if (sdk_start == sdk_kernel->my_state)
    {
        /* 服务器消息分发至上层领域 */
        if (mkernel_internal_queue_empty == (mki_err = consume_extend_data(sdk_kernel)))
            mki_err = mkernel_internal_succ;

        /* 本地消息分发至上层领域和应用 */
        consume_extend_event();

        /* 服务器消息分发至上层领域 */
        if (mkernel_internal_queue_empty == (mki_err = consume_extend_data_v3(sdk_kernel)))
            mki_err = mkernel_internal_succ;

        /* 本地消息分发至上层领域和应用 */
    }

    return mki_err;
}

mkernel_internal_error extend_load_v3(const ezdev_sdk_kernel_extend_v3 *external_extend_v3)
{
    EZDEV_SDK_UINT16 index = 0;
    for (index = 0; index < g_kernel_extend_count; index++)
    {
        if (0 == strlen(g_kernel_extend[index].kernel_extend.module))
        {
            break;
        }
        if (0 == strcmp(g_kernel_extend[index].kernel_extend.module, external_extend_v3->module))
        {
            return mkernel_internal_extend_ready;
        }
    }

    if (index == g_kernel_extend_count)
    {
        return mkernel_internal_extend_full;
    }

    memcpy(&g_kernel_extend[index].kernel_extend, external_extend_v3, sizeof(ezdev_sdk_kernel_extend_v3));

    ezdev_sdk_kernel_log_trace(mkernel_internal_succ, 0, "extend_load_v3, model_type: %s", external_extend_v3->module);

    return mkernel_internal_succ;
}


mkernel_internal_error extend_load(const ezdev_sdk_kernel_extend *external_extend)
{
    EZDEV_SDK_UINT16 index = 0;
    for (index = 0; index < g_kernel_domains_count; index++)
    {
        if (0 == g_kernel_domains[index].kernel_extend.domain_id)
        {
            break;
        }
        if (g_kernel_domains[index].kernel_extend.domain_id == external_extend->domain_id)
        {
            return mkernel_internal_extend_ready;
        }
    }

    if (index == g_kernel_domains_count)
    {
        return mkernel_internal_extend_full;
    }

    memcpy(&g_kernel_domains[index].kernel_extend, external_extend, sizeof(ezdev_sdk_kernel_extend));
    g_kernel_domains[index].domain_risk = sdk_no_risk_control;

    ezdev_sdk_kernel_log_trace(mkernel_internal_succ, 0, "extend_load %d, name:%s, version:%s", external_extend->domain_id, external_extend->extend_module_name, external_extend->extend_module_version);

    return mkernel_internal_succ;
}

void extend_load_event_notice(sdk_kernel_event_notice kernel_event_notice_cb)
{
    g_kernel_event_notice_cb = kernel_event_notice_cb;
}

mkernel_internal_error extend_serialize_sdk_version(bscJSON *pJsonRoot)
{
    char domain_id_string[16];
    EZDEV_SDK_UINT16 index = 0;
    bscJSON *pJson_sdkversion = NULL;
    pJson_sdkversion = bscJSON_CreateObject();
    if (pJson_sdkversion == NULL)
    {
        return mkernel_internal_malloc_error;
    }

    memset(domain_id_string, 0, 16);
    snprintf(domain_id_string, 16, "%d", DAS_CMD_DOMAIN);
    bscJSON_AddStringToObject(pJson_sdkversion, domain_id_string, DEV_ACCESS_DOMAIN_VERSION);

    for (; index < ezdev_sdk_extend_count; index++)
    {
        memset(domain_id_string, 0, 16);
        snprintf(domain_id_string, 16, "%d", g_kernel_domains[index].kernel_extend.domain_id);

        bscJSON_AddStringToObject(pJson_sdkversion, domain_id_string, g_kernel_domains[index].kernel_extend.extend_module_version);
    }
    bscJSON_AddItemToObject(pJsonRoot, "SDKVersion", pJson_sdkversion);

    return mkernel_internal_succ;
}

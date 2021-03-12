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


#include "lwip/netdb.h"
#include "lwip/def.h"
#include "lwip/sockets.h"
#include <platform/platform_stdlib.h>
#include "SysNet_HttpDns.h"
#include "mkernel_internal_error.h"
#include "net_platform_wrapper.h"
#include "ezdev_sdk_kernel_struct.h"

void lwipsocket_setnonblock(int socket_fd)
{
	int flag = lwip_fcntl(socket_fd, F_GETFL, 0);
	if (flag == -1)
	{
		return;
	}
	if (lwip_fcntl(socket_fd, F_SETFL, (flag | O_NONBLOCK)) == -1)
	{
		return;
	}
	return;
}

ezdev_sdk_net_work net_create(char* nic_name)
{
	freertos_net_work* freertosnet_work = NULL;
    
	freertosnet_work = (freertos_net_work*)malloc(sizeof(freertos_net_work));
	if (freertosnet_work == NULL)
	{
        goto exit;
	}
    
	freertosnet_work->lwip_fd = lwip_socket(AF_INET, SOCK_STREAM, 0);
	if (freertosnet_work->lwip_fd == -1)
	{
		free(freertosnet_work);
		freertosnet_work = NULL;
		goto exit;
	}
    
exit:
	return (ezdev_sdk_net_work)freertosnet_work;
}

ezdev_sdk_kernel_error ezdev_net_connect(ezdev_sdk_net_work net_work, const char* server_ip, int server_port,  int timeout_ms, char szRealIp[ezdev_sdk_ip_max_len])
{
	int return_value = 0;
	struct hostent   * hostent;
	struct sockaddr_in   saddr = {0};
    
	struct fd_set cnt_fdset;
	struct timeval cnt_timeout;
    char* szParseIp = NULL;
    char ip_addr[32] = {0};
    
	freertos_net_work* freertosnet_work = (freertos_net_work*)net_work;
    
	if (NULL == freertosnet_work)
	{
		return_value = mkernel_internal_input_param_invalid;
		goto exit;
	}

    hostent = gethostbyname(server_ip);
    if ( hostent == NULL ) {
        if (SysNet_TencentHttpDns(server_ip, ip_addr) != 0) {
            return_value = mkernel_internal_net_gethostbyname_error;
            goto exit;
        }
        saddr.sin_addr.s_addr = inet_addr(ip_addr);
    } else {
        saddr.sin_addr.s_addr = ((struct in_addr *)(hostent->h_addr))->s_addr;
    }
    
    saddr.sin_family    = AF_INET;
    saddr.sin_port      = lwip_htons(server_port);
    
    
#if 1        
    szParseIp = inet_ntoa(saddr.sin_addr.s_addr);
	if (strlen(szParseIp) >= ezdev_sdk_ip_max_len)
	{
		strncpy(szRealIp, szParseIp, ezdev_sdk_ip_max_len - 1);
	}
	else
	{
		strncpy(szRealIp, szParseIp, strlen(szParseIp));
	}
	
#endif
    if( lwip_connect(freertosnet_work->lwip_fd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
    {
        //		 os_printf("unable to connect to remote host %s:%d errno=%d", server_ip, server_port, errno);
        return_value = mkernel_internal_net_connect_error;
        goto exit;
    }
    
    FD_ZERO(&cnt_fdset); 
    FD_SET(freertosnet_work->lwip_fd, &cnt_fdset); 
    
    cnt_timeout.tv_sec = timeout_ms/1000; 
    cnt_timeout.tv_usec = (timeout_ms%1000)*1000;
    
    return_value = select(freertosnet_work->lwip_fd + 1, 0, &cnt_fdset, 0, &cnt_timeout);
    if (return_value <= 0)
    {
        //		 os_printf("select %s:%d returnvalue=%d ,error=%d", server_ip, server_port, return_value, errno);
        return_value = mkernel_internal_net_connect_timeout;
        goto exit;
    }
    
    return_value = 0;
exit:
	return return_value;
}

ezdev_sdk_kernel_error net_read(ezdev_sdk_net_work net_work, unsigned char* read_buf, int read_buf_maxsize, int read_timeout_ms)
{
	int ret = 0;
	int rev_size = 0;
	int rev_total_size = 0;
	struct fd_set read_fd;
	ezdev_sdk_kernel_error return_value = 0;
	struct timeval tmval = {read_timeout_ms/1000, (read_timeout_ms%1000)*1000};
    freertos_net_work* freertosnet_work = NULL;
	
    if (read_buf == NULL || read_buf_maxsize == 0) {
        goto exit;
    }
    
	freertosnet_work = (freertos_net_work*)net_work;
	if (NULL == freertosnet_work)
	{
		return_value = mkernel_internal_input_param_invalid;
		goto exit;
	}
    
	do
	{
		FD_ZERO(&read_fd);
		FD_SET(freertosnet_work->lwip_fd, &read_fd);
        
		ret = lwip_select(freertosnet_work->lwip_fd + 1, &read_fd, NULL, NULL, &tmval);
		if (ret < 0)
		{
			//socket error
			return mkernel_internal_net_socket_error;
		}
		else if (ret == 0)
		{
			//timeout
			return mkernel_internal_net_socket_timeout;
		}
        
		rev_size = recv(freertosnet_work->lwip_fd, read_buf + rev_total_size, read_buf_maxsize - rev_total_size, 0);
		if (rev_size < 0)
		{
			//socket error
			return mkernel_internal_net_socket_error;
		}
		else if (rev_size == 0)
		{
			// socket close
			return mkernel_internal_net_socket_closed;
		}
		rev_total_size += rev_size;
        
	}while(rev_total_size < read_buf_maxsize);
    
    //	os_printf("net_read fd:%d end result:%d rev_total_size:%d , read_buf_maxsize:%d\n", freertosnet_work->lwip_fd, return_value, rev_total_size, read_buf_maxsize);
exit:
	return return_value;
}

ezdev_sdk_kernel_error net_write(ezdev_sdk_net_work net_work, unsigned char* write_buf, int write_buf_size, int send_timeout_ms, int* real_write_buf_size)
{
	int ret = 0;
	int send_size = 0;
	int send_total_size = 0;
	ezdev_sdk_kernel_error return_value = 0;
    
	fd_set write_fd;
	struct timeval tmval = {send_timeout_ms/1000, (send_timeout_ms%1000)*1000};
    
	freertos_net_work* freertosnet_work = (freertos_net_work*)net_work;
	if (NULL == freertosnet_work || NULL == real_write_buf_size)
	{
		return_value = mkernel_internal_input_param_invalid;
		goto exit;
	}
	
	do 
	{
		FD_ZERO(&write_fd);
		FD_SET(freertosnet_work->lwip_fd, &write_fd);
        
		ret = select(freertosnet_work->lwip_fd + 1,NULL , &write_fd, NULL, &tmval);
		if (ret < 0)
		{
			//socket error
			return mkernel_internal_net_socket_error;
		}
		else if (ret == 0)
		{
			//timeout
			return mkernel_internal_net_socket_timeout;
		}
        
		send_size = send(freertosnet_work->lwip_fd, write_buf + send_total_size, write_buf_size - send_total_size, 0);
		if (send_size == -1)
		{
			//socket error
			return mkernel_internal_net_socket_error;
		}
        // 		else if (send_size <  write_buf_size - send_total_size)
        // 		{
        // 			//send buf full
        // 			return ezdev_sdk_kernel_net_send_buf_full;
        // 		}
        
		*real_write_buf_size = send_size;

	} while(0);
	
//	os_printf("net_read fd:%d end result:%d send_total_size:%d , write_buf_size:%d\n", freertosnet_work->lwip_fd, return_value, send_total_size, write_buf_size);
exit:
	return return_value;
}

void net_disconnect(ezdev_sdk_net_work net_work)
{
	freertos_net_work* freertosnet_work = (freertos_net_work*)net_work;
	if (NULL == freertosnet_work)
	{
		return;
	}
    
	lwip_close(freertosnet_work->lwip_fd);
	freertosnet_work->lwip_fd = -1;
}

void net_destroy(ezdev_sdk_net_work net_work)
{
	freertos_net_work* freertosnet_work = (freertos_net_work*)net_work;
	if (NULL == freertosnet_work)
	{
		return;
	}
	free(freertosnet_work);
}

int net_getsocket(ezdev_sdk_net_work net_work)
{
	freertos_net_work* freertosnet_work = (freertos_net_work*)net_work;
	if (NULL == freertosnet_work)
	{
		return -1;
	}
	

	return freertosnet_work->lwip_fd;
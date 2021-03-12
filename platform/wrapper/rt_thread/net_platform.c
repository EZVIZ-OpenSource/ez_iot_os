
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include<net/if.h>
#include "ezdev_sdk_kernel_struct.h"
#include "mkernel_internal_error.h"
#include "net_platform_wrapper.h"


void signal_callback_handler(int signum){
    printf("-----------Caught signal SIGPIPE----------- %d\n", signum);
}

char isIPAddress(const char *s)  
{  
	const char *pChar;  
	char rv = 1;  
	int tmp1, tmp2, tmp3, tmp4, i;  

	while( 1 )  
	{  
		i = sscanf(s, "%d.%d.%d.%d", &tmp1, &tmp2, &tmp3, &tmp4);  

		if( i != 4 )  
		{  
			rv = 0;  
			break;  
		}  

		if( (tmp1 > 255) || (tmp2 > 255) || (tmp3 > 255) || (tmp4 > 255) )  
		{  
			rv = 0;  
			break;  
		}  

		for( pChar = s; *pChar != 0; pChar++ )  
		{  
			if( (*pChar != '.')  
				&& ((*pChar < '0') || (*pChar > '9')) )  
			{  
				rv = 0;  
				break;  
			}  
		}  
		break;  
	}  

	return rv;  
}  
static in_addr_t ParseHost(const char* host, char szRealIp[ezdev_sdk_ip_max_len])
{
	
	char str[128]= {0}; 
	int ret, herr;
	char buf[1024];
	struct hostent entry, *hp;

	if (host == NULL || strlen(host) == 0) {
		return htonl(INADDR_ANY);
	}
	ret = gethostbyname_r(host, &entry, buf, 1024, &hp, &herr);
	if (ret || hp == NULL) {
		printf("gethostbyname_r failed , ret:%d, %d, errno:%d\n", ret, hp, herr);
		return INADDR_NONE;
	}

	const char* szParseIp = inet_ntop(entry.h_addrtype, entry.h_addr, str, sizeof(str));
	if(NULL == szParseIp)
	{
		printf("inet_ntop ParseIp failed, szParseIp is null \n");
		return INADDR_NONE;
	}
	if (strlen(szParseIp) >= ezdev_sdk_ip_max_len)
	{
		strncpy(szRealIp, szParseIp, ezdev_sdk_ip_max_len - 1);
	}
	else
	{
		strncpy(szRealIp, szParseIp, strlen(szParseIp));
	}
	printf("host parser:address:%s\n", szParseIp);
	return ((struct in_addr*)(entry.h_addr))->s_addr;

}

void linuxsocket_setnonblock(int socket_fd)
{
	int flag = fcntl(socket_fd, F_GETFL, 0);
	if (flag == -1)
	{
		return;
	}
	if (fcntl(socket_fd, F_SETFL, (flag | O_NONBLOCK)) == -1)
	{
		return;
	}
	return;
}

mkernel_internal_error linuxsocket_poll(int socket_fd, POLL_TYPE type, int timeout)
{
	struct pollfd poll_fd;
	int nfds = 0;

	poll_fd.fd = socket_fd;
	poll_fd.events = type;
	poll_fd.revents = 0;
	
	if (socket_fd < 0) {
		return mkernel_internal_input_param_invalid;
	}

	nfds = poll(&poll_fd, 1, timeout);
	if (nfds < 0) 
	{
//		DERROR("poll error, errno %d\n", errno);
		if (errno == EINTR) 
		{
			return mkernel_internal_succ;
		} 
		else
		{
			return mkernel_internal_net_socket_error;
		}
	}
	else if (nfds > 0) 
	{
		if (poll_fd.revents & type) 
		{ // prior to check error
			return mkernel_internal_succ;
		} 
		else if (poll_fd.revents & (POLLNVAL | POLLERR | POLLHUP)) 
		{
			return mkernel_internal_net_socket_error;
		} 
		else 
		{
			return mkernel_internal_net_socket_error;
		}
	} 
	else 
	{
		// timeout
		return mkernel_internal_net_socket_timeout;
	}
}

/** 
 *  \brief		网络连接的创建,默认为TCP协议
 *  \method		net_create
 *  \param[in] 	nic_name	网卡名称，如果nic_name不为空或指向的地址是有效值，创建的socket将绑定这个网卡
 *  \return 	成功返回网络连接上下文 失败返回NULL
 */
ezdev_sdk_net_work net_create(char* nic_name)
{
	//signal(SIGPIPE, signal_callback_handler);

	linux_net_work* linuxnet_work = NULL;
	const int opt = 1400;
	int ret = 0;
	struct ifreq ifr = {0};

	linuxnet_work = (linux_net_work*)malloc(sizeof(linux_net_work));
	if (linuxnet_work == NULL)
	{
		return NULL;
	}

	linuxnet_work->socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (linuxnet_work->socket_fd == -1)
	{
		free(linuxnet_work);
		return NULL;
	}

	ret = setsockopt(linuxnet_work->socket_fd , IPPROTO_TCP, TCP_MAXSEG, &opt, sizeof(opt));
	if (ret < 0) 
	{
		printf("set socket opt, TCP_MAXSEG error\n");
	}

	if(nic_name && strlen(nic_name) > 0)
	{
		strncpy(ifr.ifr_name, nic_name, sizeof(ifr.ifr_name));

		ret = setsockopt(linuxnet_work->socket_fd, SOL_SOCKET, SO_BINDTODEVICE,  (void*)&ifr, sizeof(ifr));
		if (ret < 0) 
		{
			printf("set socket opt, SO_BINDTODEVICE error\n");
		}
	}

	return (ezdev_sdk_net_work)linuxnet_work;
}

mkernel_internal_error net_connect(ezdev_sdk_net_work net_work, const char* server_ip, int server_port,  int timeout_ms, char szRealIp[ezdev_sdk_ip_max_len])
{
	struct sockaddr_in dst_addr;
	int return_value = 0;
    struct fd_set cnt_fdset;
	struct timeval cnt_timeout;
	linux_net_work* linuxnet_work = (linux_net_work*)net_work;

	int socket_err = 0;
	socklen_t socklen = sizeof(socket_err);

	if (NULL == linuxnet_work)
	{
		return mkernel_internal_input_param_invalid;
	}

	linuxsocket_setnonblock(linuxnet_work->socket_fd);

	dst_addr.sin_family=AF_INET;
	dst_addr.sin_addr.s_addr=ParseHost(server_ip, szRealIp);
	dst_addr.sin_port=htons(server_port);
	if (connect(linuxnet_work->socket_fd, (const struct sockaddr *)(&dst_addr), sizeof(dst_addr)) == -1 && errno != EINPROGRESS)
	{
		return mkernel_internal_net_connect_error;
	}

    FD_ZERO(&cnt_fdset); 
    FD_SET(linuxnet_work->socket_fd, &cnt_fdset); 

    cnt_timeout.tv_sec = timeout_ms/1000; 
    cnt_timeout.tv_usec = (timeout_ms%1000)*1000;

    return_value = select(linuxnet_work->socket_fd + 1, 0, &cnt_fdset, 0, &cnt_timeout);
    if (return_value <= 0)
    {
        printf("select %s:%d returnvalue=%d ,error=%d", server_ip, server_port, return_value, errno);
        return_value = mkernel_internal_net_connect_timeout;
        goto exit;
    }

    return_value = 0;
     
exit:
	return return_value;
}

mkernel_internal_error net_read(ezdev_sdk_net_work net_work, unsigned char* read_buf, int read_buf_maxsize, int read_timeout_ms)
{
    int ret;
	int rev_size = 0;
	int rev_total_size = 0;
	mkernel_internal_error return_value = 0;
    struct fd_set read_fd;
	struct timeval tmval = {read_timeout_ms/1000, (read_timeout_ms%1000)*1000};
	
	linux_net_work* linuxnet_work = (linux_net_work*)net_work;
	if (NULL == linuxnet_work)
	{
		return mkernel_internal_input_param_invalid;
	}

	do
	{
	    FD_ZERO(&read_fd);
		FD_SET(linuxnet_work->socket_fd, &read_fd);

		ret = select(linuxnet_work->socket_fd + 1, &read_fd, NULL, NULL, &tmval);
		if (ret < 0)
		{
			//socket error  or  socket close
			return return_value;
		}
		else if (ret == 0)
		{
			//timeout
			return mkernel_internal_net_socket_timeout;
		}

		rev_size = recv(linuxnet_work->socket_fd, read_buf + rev_total_size, read_buf_maxsize - rev_total_size, 0);
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


	return mkernel_internal_succ;
}

mkernel_internal_error net_write(ezdev_sdk_net_work net_work, unsigned char* write_buf, int write_buf_size, int send_timeout_ms, int* real_write_buf_size)
{
    int ret = -1;
	int send_size = 0;
	int send_total_size = 0;

	mkernel_internal_error return_value = 0;
    fd_set write_fd;
	struct timeval tmval = {send_timeout_ms/1000, (send_timeout_ms%1000)*1000};

	linux_net_work* linuxnet_work = (linux_net_work*)net_work;
	if (NULL == linuxnet_work)
	{
		return mkernel_internal_input_param_invalid;
	}
	
	do 
	{
        FD_ZERO(&write_fd);
		FD_SET(linuxnet_work->socket_fd, &write_fd);

		ret = select(linuxnet_work->socket_fd + 1,NULL , &write_fd, NULL, &tmval);
		if (ret < 0)
		{
			//socket error  or  socket close
			return return_value;
		}
		else if (ret == 0)
		{
			return mkernel_internal_net_socket_timeout;
		}

		send_size = send(linuxnet_work->socket_fd, write_buf + send_total_size, write_buf_size - send_total_size, 0);
		if (send_size == -1)
		{
			//socket error
			return mkernel_internal_net_socket_error;
		}
// 		else if (send_size <  write_buf_size - send_total_size)
// 		{
// 			//send buf full
// 			return mkernel_internal_net_send_buf_full;
// 		}
		
		*real_write_buf_size = send_size;

	} while(0);

	return mkernel_internal_succ;
}

void net_disconnect(ezdev_sdk_net_work net_work)
{
	linux_net_work* linuxnet_work = (linux_net_work*)net_work;
	if (NULL == linuxnet_work)
	{
		return;
	}
	close(linuxnet_work->socket_fd);
	linuxnet_work->socket_fd = 0;
}

void net_destroy(ezdev_sdk_net_work net_work)
{
	linux_net_work* linuxnet_work = (linux_net_work*)net_work;
	if (NULL == linuxnet_work)
	{
		return;
	}
	free(linuxnet_work);
	return;
}

int net_getsocket(ezdev_sdk_net_work net_work)
{
	linux_net_work* linuxnet_work = (linux_net_work*)net_work;
	if (NULL == linuxnet_work)
	{
		return -1;
	}
	

	return linuxnet_work->socket_fd;
}
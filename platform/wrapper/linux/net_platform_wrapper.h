#ifndef H_NET_PLATFORM_WRAPPER_H_
#define H_NET_PLATFORM_WRAPPER_H_
#include <poll.h>
/**
 * \brief   linux й╣ож
 */
typedef struct 
{
	int socket_fd;
}linux_net_work;


typedef enum  
{
	POLL_RECV = POLLIN,
	POLL_SEND = POLLOUT,
	POLL_ALL = POLLIN | POLLOUT,
}POLL_TYPE;

#endif
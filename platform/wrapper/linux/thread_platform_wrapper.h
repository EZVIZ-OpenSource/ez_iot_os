#ifndef H_THREAD_PLATFORM_SWAP_H_
#define H_THREAD_PLATFORM_SWAP_H_

#include <pthread.h>

/**
 * \brief   linux й╣ож
 */

typedef struct 
{
	pthread_mutex_t lock;
}sdk_mutex_platform;

typedef struct thread_handle_platform
{
	pthread_t thread_hd;
	void* thread_arg;
	unsigned int (*task_do)(void * user_data);
	char thread_name[16];
}thread_handle;

#endif
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
#include <stdio.h>
#include <string.h>

int g_testcount = 0;
int get_devinfo_fromconfig(const char* path, char* devinfo_context, int devinfo_context_len)
{
	int return_code = 0;
	int real_read = 0;
	int total_read = 0;
	unsigned char block[1024];
	FILE* config_file = NULL;
	
	config_file = fopen(path, "r");
	if (config_file == NULL)
	{
		return_code = -1;
		return return_code;
	}
	g_testcount++;
	do 
	{
		memset(block, 0, 1024);

		real_read = fread(block, 1, 1024, config_file);
		if (real_read > 1024 || real_read <= 0)
		{
			return_code = -1;
			break;
		}

		if (total_read + real_read >= devinfo_context_len)
		{
			return_code = -1;
			break;
		}
		memcpy(devinfo_context+total_read, block, real_read);
		total_read += real_read;

	} while (!feof(config_file));

	if (config_file != NULL)
	{
		fclose(config_file);
		config_file = NULL;
	}

	return return_code;
}

#ifdef _RT_THREAD_
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

static int file_writen(int fd, const char* buf, size_t count)
{
    if (fd < 0) return -1;

    int ret = 0;
    size_t left = count;

    while (left > 0) {
        if ((ret = write(fd, buf, left)) < 0) {
            if (errno == EINTR) {
                continue;
            }

            perror("writen error");
            if (left == count) {
                return -1;
            }
            break;
        } else if (ret == 0) {
            break;
        } else {
            buf += ret;
            left -= ret;
        }
    }

    return (count - left);

}

static int file_readn(int fd, char* buf, size_t count)
{
    if (fd < 0) return -1;

    int ret = 0;
    size_t left = count;

    while (left > 0) {
        if ((ret = read(fd, buf, left)) < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("readn error");
            if (left == count) {
                return -1;
            }
            break;
        } else if (ret == 0) {
            break;
        } else {
            buf += ret;
            left -= ret;
        }
    }

    return (count - left);
}

int set_file_value(const char* path, unsigned char* keyvalue, int keyvalue_size)
{
	int return_code = 0;
	int real_write = 0;
	int fd = -1;
	fd = open(path, O_WRONLY | O_CREAT, 0744);//create file
	if (fd == -1) 
	{
		return -1;
	}
	
	do 
	{
		real_write = file_writen(fd, keyvalue, keyvalue_size);
		if (real_write != keyvalue_size)
		{
			printf("write buf to fd error len%d\n", keyvalue_size);
			return_code = -1;
			break;
		}
		fdatasync(fd); // more effcient than fsync
	} while (0);

	if (fd)
	{
		close(fd);
	}

	return return_code;
}

int get_file_value(const char* path, unsigned char* keyvalue, int keyvalue_maxsize)
{
	int return_code = 0;
	int real_read = 0;
	unsigned char block[64];
	int fd = -1;
	fd = open(path, O_WRONLY | O_CREAT, 0744);
	if (fd == -1) 
	{
		return -1;
	}

	do 
	{	
		memset(block, 0, 64);
		real_read = file_readn(fd, block, 64);
		if (real_read > 64 || real_read <= 0 )
		{
			return_code = -1;
			break;
		}

		if (real_read > keyvalue_maxsize)
		{
			return_code = -1;
			printf("read error len %d\n", real_read);
			break;
		}
		memcpy(keyvalue, block, real_read);
	} while (0);

	if (fd != -1)
	{
		close(fd);
	}

	return return_code;
}

#else

int set_file_value(const char* path, unsigned char* keyvalue, int keyvalue_size)
{
	FILE* config_file = NULL;
	int return_code = 0;
	int real_write = 0;
	config_file = fopen(path, "w+");
	if (config_file == NULL)
	{
		return_code = -1;
		return return_code;
	}
	
	do 
	{
		real_write = fwrite(keyvalue, 1, keyvalue_size, config_file);
		if (real_write != keyvalue_size)
		{
			return_code = -1;
			break;
		}
		fflush(config_file);
	} while (0);

	if (config_file != NULL)
	{
		fclose(config_file);
		config_file = NULL;
	}

	return return_code;
}

int get_file_value(const char* path, unsigned char* keyvalue, int keyvalue_maxsize)
{
	int return_code = 0;
	int real_read = 0;
	unsigned char block[64];
	FILE* config_file = NULL;
	config_file = fopen(path, "r");  
	if (config_file == NULL)
	{
		return_code = -1;
		return return_code;
	}

	do 
	{
		memset(block, 0, 64);
		real_read = fread(block, 1, 64, config_file);
		if (real_read > 64 || real_read <= 0 )
		{
			return_code = -1;
			break;
		}

		if (real_read > keyvalue_maxsize)
		{
			return_code = -1;
			break;
		}
		memcpy(keyvalue, block, real_read);
	} while (0);

	if (config_file != NULL)
	{
		fclose(config_file);
		config_file = NULL;
	}

	return return_code;
}
#endif
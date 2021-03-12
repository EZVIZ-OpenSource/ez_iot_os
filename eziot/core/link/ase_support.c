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
#include "ase_support.h"
#include "mbedtls/aes.h"
#include "mbedtls/gcm.h"
#include "mkernel_internal_error.h"
#include "base_typedef.h"


#define iv_len  12
#define add_len 16
#define tag_len 16


EZDEV_SDK_UINT32 calculate_padding_len(EZDEV_SDK_UINT32 len)
{
	EZDEV_SDK_UINT32 input_padding_len = 0;
	input_padding_len = (0x10 - (len & 0x0F));
	input_padding_len += len;
	return input_padding_len;
}

mkernel_internal_error aes_cbc_128_dec_padding(const unsigned char aes_key[16], 
											   const unsigned char *input_buf, EZDEV_SDK_UINT32 input_length, 
											   unsigned char *output_buf, EZDEV_SDK_UINT32 *output_buf_len)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	EZDEV_SDK_INT32 aes_result = 0;
	unsigned char iv[16];
	bscomptls_aes_context aes_ctx;
	EZDEV_SDK_UINT32 index = 0;
	unsigned char padding_char = 0;
	unsigned char other_char = 0;

	do 
	{
		bscomptls_aes_init( &aes_ctx );
		memset((void*)iv, 0, 16);
		for(index=0;index<8;index++)
		{
			iv[index]=index + 0x30;
		}

		if (input_length==0 || input_length%16 != 0)
		{
			sdk_error = mkernel_internal_aes_input_len;
			break;
		}

		aes_result = bscomptls_aes_setkey_dec(&aes_ctx, aes_key, 128);
		if (aes_result != 0)
		{
			sdk_error = mkernel_internal_casll_mbedtls_setdeckey_error;
			break;
		}
		aes_result = bscomptls_aes_crypt_cbc(&aes_ctx, BSCOMPTLS_AES_DECRYPT, input_length, iv, input_buf, output_buf);
		if (aes_result != 0)
		{
			sdk_error = mkernel_internal_casll_mbedtls_crypt_error;
			break;
		}

		padding_char = output_buf[input_length-1];

		if (padding_char > 16 || padding_char <= 0)
		{
			sdk_error = mkernel_internal_aes_padding_unmatched;
			break;
		}

		for (index=1; index<=(EZDEV_SDK_UINT32)padding_char; index++)
		{
			other_char = output_buf[input_length - index];
			if (other_char != padding_char)
			{
				break;
			}
		}

		if (other_char != padding_char)
		{
			sdk_error = mkernel_internal_aes_padding_unmatched;
			break;
		}

		*output_buf_len = (input_length - (int)padding_char);

	} while (0);
	
	bscomptls_aes_free(&aes_ctx);
	return sdk_error;
}

void buf_padding(unsigned char* buf, EZDEV_SDK_INT32 padding_len ,EZDEV_SDK_INT32 len)
{
	EZDEV_SDK_INT32 i=0;
	EZDEV_SDK_INT32 padding_char = 0;

	padding_char = (padding_len - len);

	for(i=len; i < padding_len; i++)
	{
		buf[i] = (unsigned char)padding_char;
	}

	return ;
}

mkernel_internal_error aes_cbc_128_enc_padding(const unsigned char aes_key[16], \
											   unsigned char *input_buf, EZDEV_SDK_UINT32 input_length, EZDEV_SDK_UINT32 input_length_padding, \
											   unsigned char *output_buf, EZDEV_SDK_UINT32 *output_length)
{
	mkernel_internal_error sdk_error = mkernel_internal_succ;
	EZDEV_SDK_INT32 aes_result = 0;
	unsigned char iv[16];
	bscomptls_aes_context aes_ctx;
	EZDEV_SDK_INT32 i=0;
	unsigned char padding_char = 0;

	padding_char = (input_length_padding - input_length);

	for(i=input_length; i < input_length_padding; i++)
	{
		input_buf[i] = padding_char;
	}

	do 
	{
		bscomptls_aes_init( &aes_ctx );
		memset(iv, 0, 16);
		for(i=0;i<8;i++)
		{
			iv[i]=i + 0x30;
		}

		aes_result = bscomptls_aes_setkey_enc(&aes_ctx, aes_key, 128);
		if (aes_result != 0)
		{
            sdk_error = mkernel_internal_casll_mbedtls_setdeckey_error;
			break;
		}
		aes_result = bscomptls_aes_crypt_cbc(&aes_ctx, BSCOMPTLS_AES_ENCRYPT, input_length_padding, iv, input_buf, output_buf);
		if (aes_result != 0)
		{
            sdk_error = mkernel_internal_casll_mbedtls_crypt_error;
			break;
		}
	} while (0);

	bscomptls_aes_free(&aes_ctx);

	if (sdk_error == mkernel_internal_succ)
	{
		*output_length = input_length_padding;
	}

	return sdk_error;
}


// static int timer_entropy_poll(void *data, uint8_t *output, size_t len, size_t *olen)
// {
//     unsigned long timer = bscomptls_timing_hardclock();
//     ((void)data);
//     *olen = 0;
// 
//     if (len < sizeof(unsigned long))
//         return(0);
// 
//     memcpy(output, &timer, sizeof(unsigned long));
//     *olen = sizeof(unsigned long);
// 
//     return(0);
// }


// static int GenRandom(const uint8_t *custom, uint8_t * out, size_t out_len)
// {
//     mkernel_internal_error sdk_error = mkernel_internal_succ;
//     int ret = 0;
//     bscomptls_entropy_context entropy;
//     bscomptls_ctr_drbg_context ctr_drbg;
// 
//     do 
//     {
//         bscomptls_entropy_init(&entropy);
//         bscomptls_ctr_drbg_init(&ctr_drbg);
// 
//         bscomptls_entropy_add_source(&entropy, timer_entropy_poll, NULL, BSCOMPTLS_ENTROPY_MAX_GATHER, BSCOMPTLS_ENTROPY_SOURCE_STRONG);
// 
//         sdk_error = bscomptls_ctr_drbg_seed(&ctr_drbg, bscomptls_entropy_func, &entropy, custom, strlen(custom));
//         if (sdk_error != 0)
//         {
//             sdk_error = mkernel_internal_casll_mbedtls_setdeckey_error;
//             break;
//         }
// 
//         sdk_error = bscomptls_ctr_drbg_random(&ctr_drbg, out, out_len);
//         if (sdk_error != 0)
//         {
//             sdk_error = mkernel_internal_casll_mbedtls_setdeckey_error;
//             break;
//         }
//     } while (0);
// 
//     bscomptls_ctr_drbg_free(&ctr_drbg);
//     bscomptls_entropy_free(&entropy);
// 
//     return sdk_error;
// }


//明文input_buf不需要用户填充补齐，gcm算法内部自动处理，生成的密文output_buf也是不包含填充内容的，即全部是密文内容
mkernel_internal_error aes_gcm_128_enc_padding(const unsigned char gcm_key[16], \
                                                unsigned char *input_buf, EZDEV_SDK_UINT32 input_length, \
                                                unsigned char *output_buf, EZDEV_SDK_UINT32 *output_length, \
                                                unsigned char* output_tag_buf, EZDEV_SDK_UINT32 tag_buf_len)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    EZDEV_SDK_INT32 gcm_result = 0;
    EZDEV_SDK_INT32 index = 0;
    bscomptls_gcm_context gcm_ctx;
    unsigned int key_len = 16 * 8;
    bscomptls_cipher_id_t cipher = BSCOMPTLS_CIPHER_ID_AES;
    unsigned char iv[iv_len];

//    //调试代码，需要时候放开
//     unsigned char gcm_key_hex[16 * 2 + 1] = { 0 };
//     unsigned char iv_hex[iv_len * 2 + 1] = { 0 };
//     unsigned char input_buf_hex[512];
//     unsigned char output_buf_hex[512];
//     unsigned char tag_buf_hex[32 + 1] = { 0 };

    do
    {
        bscomptls_gcm_init(&gcm_ctx);
        memset(iv, 0, iv_len);
        for (index = 0; index < iv_len / 2; index++)
        {
            iv[index] = index + 0x30;
        }

        gcm_result = bscomptls_gcm_setkey(&gcm_ctx, cipher, gcm_key, key_len);
        if (gcm_result != 0)
        {
            sdk_error = mkernel_internal_casll_mbedtls_setdeckey_error;
            break;
        }

        //GenRandom("mbedtls_IV", iv, iv_len);

//         //调试代码，需要时候放开
//         bscomptls_hexdump(gcm_key, 16, 1, gcm_key_hex);
//         bscomptls_hexdump(iv, iv_len, 1, iv_hex);
//         bscomptls_hexdump(input_buf, input_length, 1, input_buf_hex);

        gcm_result = bscomptls_gcm_crypt_and_tag(&gcm_ctx, BSCOMPTLS_GCM_ENCRYPT, input_length/*input_length_padding*/, iv, iv_len, NULL/*addition*/, 0/*add_len*/, input_buf, output_buf, tag_buf_len, output_tag_buf);
        if (gcm_result != 0)
        {
            sdk_error = mkernel_internal_casll_mbedtls_crypt_error;
            break;
        }

//         //调试代码，需要时候放开
//         bscomptls_hexdump(output_tag_buf, tag_buf_len, 1, tag_buf_hex);
//         bscomptls_hexdump(output_buf, input_length, 1, output_buf_hex);

    } while (0);

    bscomptls_gcm_free(&gcm_ctx);

    if (sdk_error == mkernel_internal_succ)
    {
        //output_buf内容有可能含有0，不能用*output_length = strlen(output_buf);
        *output_length = input_length;
    }

    return sdk_error;
}


//密文input_buf不需要用户填充补齐，gcm算法内部自动处理，生成的明文output_buf也是不包含填充内容的，即全部是明文内容
mkernel_internal_error aes_gcm_128_dec_padding(const unsigned char gcm_key[16], \
                                                const unsigned char *input_buf, EZDEV_SDK_UINT32 input_length, \
                                                unsigned char *output_buf, EZDEV_SDK_UINT32 *output_buf_len, \
                                                unsigned char* input_tag_buf, EZDEV_SDK_UINT32 tag_buf_len)
{
    mkernel_internal_error sdk_error = mkernel_internal_succ;
    EZDEV_SDK_INT32 gcm_result = 0;
    unsigned char iv[iv_len];
    bscomptls_gcm_context gcm_ctx;
    bscomptls_cipher_id_t cipher = BSCOMPTLS_CIPHER_ID_AES;
    unsigned int key_len = 128;
    //static const uint8_t addition[add_len] = { 0xfe, 0xa5, 0x5a, 0xef };
    EZDEV_SDK_UINT32 index = 0;

    do
    {
        bscomptls_gcm_init(&gcm_ctx);
        memset((void*)iv, 0, iv_len);
        for (index = 0; index < iv_len / 2; index++)
        {
            iv[index] = index + 0x30;
        }

        if (input_length == 0/* || input_length % 16 != 0*/)
        {
            sdk_error = mkernel_internal_aes_input_len;
            break;
        }

        gcm_result = bscomptls_gcm_setkey(&gcm_ctx, cipher, gcm_key, key_len);
        if (gcm_result != 0)
        {
            sdk_error = mkernel_internal_casll_mbedtls_setdeckey_error;
            break;
        }

        gcm_result = bscomptls_gcm_auth_decrypt(&gcm_ctx, input_length, iv, iv_len, NULL/*addition*/, 0/*add_len*/, input_tag_buf, tag_buf_len, input_buf, output_buf);
        if (gcm_result != 0)
        {
            sdk_error = mkernel_internal_casll_mbedtls_crypt_error;
            break;
        }
    } while (0);

    bscomptls_gcm_free(&gcm_ctx);

    if (sdk_error == mkernel_internal_succ)
    {
        //output_buf内容有可能含有0，不能用*output_buf_len = strlen(output_buf);
        *output_buf_len = input_length;
    }

    return sdk_error;
}




/**
 * \file md_wrap.c
 *
 * \brief Generic message digest wrapper for mbed TLS
 *
 * \author Adriaan de Jong <dejong@fox-it.com>
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#if !defined(BSCOMPTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include BSCOMPTLS_CONFIG_FILE
#endif

#if defined(BSCOMPTLS_MD_C)

#include "mbedtls/md_internal.h"

#if defined(BSCOMPTLS_MD2_C)
#include "mbedtls/md2.h"
#endif

#if defined(BSCOMPTLS_MD4_C)
#include "mbedtls/md4.h"
#endif

#if defined(BSCOMPTLS_MD5_C)
#include "mbedtls/md5.h"
#endif

#if defined(BSCOMPTLS_RIPEMD160_C)
#include "mbedtls/ripemd160.h"
#endif

#if defined(BSCOMPTLS_SHA1_C)
#include "mbedtls/sha1.h"
#endif

#if defined(BSCOMPTLS_SHA256_C)
#include "mbedtls/sha256.h"
#endif

#if defined(BSCOMPTLS_SHA512_C)
#include "mbedtls/sha512.h"
#endif

#if defined(BSCOMPTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdlib.h>
#define bscomptls_calloc    calloc
#define bscomptls_free       free
#endif

#if defined(BSCOMPTLS_MD2_C)

static void md2_starts_wrap( void *ctx )
{
    bscomptls_md2_starts( (bscomptls_md2_context *) ctx );
}

static void md2_update_wrap( void *ctx, const unsigned char *input,
                             size_t ilen )
{
    bscomptls_md2_update( (bscomptls_md2_context *) ctx, input, ilen );
}

static void md2_finish_wrap( void *ctx, unsigned char *output )
{
    bscomptls_md2_finish( (bscomptls_md2_context *) ctx, output );
}

static void *md2_ctx_alloc( void )
{
    void *ctx = bscomptls_calloc( 1, sizeof( bscomptls_md2_context ) );

    if( ctx != NULL )
        bscomptls_md2_init( (bscomptls_md2_context *) ctx );

    return( ctx );
}

static void md2_ctx_free( void *ctx )
{
    bscomptls_md2_free( (bscomptls_md2_context *) ctx );
    bscomptls_free( ctx );
}

static void md2_clone_wrap( void *dst, const void *src )
{
    bscomptls_md2_clone( (bscomptls_md2_context *) dst,
                 (const bscomptls_md2_context *) src );
}

static void md2_process_wrap( void *ctx, const unsigned char *data )
{
    ((void) data);

    bscomptls_md2_process( (bscomptls_md2_context *) ctx );
}

const bscomptls_md_info_t bscomptls_md2_info = {
    BSCOMPTLS_MD_MD2,
    "MD2",
    16,
    16,
    md2_starts_wrap,
    md2_update_wrap,
    md2_finish_wrap,
    bscomptls_md2,
    md2_ctx_alloc,
    md2_ctx_free,
    md2_clone_wrap,
    md2_process_wrap,
};

#endif /* BSCOMPTLS_MD2_C */

#if defined(BSCOMPTLS_MD4_C)

static void md4_starts_wrap( void *ctx )
{
    bscomptls_md4_starts( (bscomptls_md4_context *) ctx );
}

static void md4_update_wrap( void *ctx, const unsigned char *input,
                             size_t ilen )
{
    bscomptls_md4_update( (bscomptls_md4_context *) ctx, input, ilen );
}

static void md4_finish_wrap( void *ctx, unsigned char *output )
{
    bscomptls_md4_finish( (bscomptls_md4_context *) ctx, output );
}

static void *md4_ctx_alloc( void )
{
    void *ctx = bscomptls_calloc( 1, sizeof( bscomptls_md4_context ) );

    if( ctx != NULL )
        bscomptls_md4_init( (bscomptls_md4_context *) ctx );

    return( ctx );
}

static void md4_ctx_free( void *ctx )
{
    bscomptls_md4_free( (bscomptls_md4_context *) ctx );
    bscomptls_free( ctx );
}

static void md4_clone_wrap( void *dst, const void *src )
{
    bscomptls_md4_clone( (bscomptls_md4_context *) dst,
                 (const bscomptls_md4_context *) src );
}

static void md4_process_wrap( void *ctx, const unsigned char *data )
{
    bscomptls_md4_process( (bscomptls_md4_context *) ctx, data );
}

const bscomptls_md_info_t bscomptls_md4_info = {
    BSCOMPTLS_MD_MD4,
    "MD4",
    16,
    64,
    md4_starts_wrap,
    md4_update_wrap,
    md4_finish_wrap,
    bscomptls_md4,
    md4_ctx_alloc,
    md4_ctx_free,
    md4_clone_wrap,
    md4_process_wrap,
};

#endif /* BSCOMPTLS_MD4_C */

#if defined(BSCOMPTLS_MD5_C)

static void md5_starts_wrap( void *ctx )
{
    bscomptls_md5_starts( (bscomptls_md5_context *) ctx );
}

static void md5_update_wrap( void *ctx, const unsigned char *input,
                             size_t ilen )
{
    bscomptls_md5_update( (bscomptls_md5_context *) ctx, input, ilen );
}

static void md5_finish_wrap( void *ctx, unsigned char *output )
{
    bscomptls_md5_finish( (bscomptls_md5_context *) ctx, output );
}

static void *md5_ctx_alloc( void )
{
    void *ctx = bscomptls_calloc( 1, sizeof( bscomptls_md5_context ) );

    if( ctx != NULL )
        bscomptls_md5_init( (bscomptls_md5_context *) ctx );

    return( ctx );
}

static void md5_ctx_free( void *ctx )
{
    bscomptls_md5_free( (bscomptls_md5_context *) ctx );
    bscomptls_free( ctx );
}

static void md5_clone_wrap( void *dst, const void *src )
{
    bscomptls_md5_clone( (bscomptls_md5_context *) dst,
                 (const bscomptls_md5_context *) src );
}

static void md5_process_wrap( void *ctx, const unsigned char *data )
{
    bscomptls_md5_process( (bscomptls_md5_context *) ctx, data );
}

const bscomptls_md_info_t bscomptls_md5_info = {
    BSCOMPTLS_MD_MD5,
    "MD5",
    16,
    64,
    md5_starts_wrap,
    md5_update_wrap,
    md5_finish_wrap,
    bscomptls_md5,
    md5_ctx_alloc,
    md5_ctx_free,
    md5_clone_wrap,
    md5_process_wrap,
};

#endif /* BSCOMPTLS_MD5_C */

#if defined(BSCOMPTLS_RIPEMD160_C)

static void ripemd160_starts_wrap( void *ctx )
{
    bscomptls_ripemd160_starts( (bscomptls_ripemd160_context *) ctx );
}

static void ripemd160_update_wrap( void *ctx, const unsigned char *input,
                                   size_t ilen )
{
    bscomptls_ripemd160_update( (bscomptls_ripemd160_context *) ctx, input, ilen );
}

static void ripemd160_finish_wrap( void *ctx, unsigned char *output )
{
    bscomptls_ripemd160_finish( (bscomptls_ripemd160_context *) ctx, output );
}

static void *ripemd160_ctx_alloc( void )
{
    void *ctx = bscomptls_calloc( 1, sizeof( bscomptls_ripemd160_context ) );

    if( ctx != NULL )
        bscomptls_ripemd160_init( (bscomptls_ripemd160_context *) ctx );

    return( ctx );
}

static void ripemd160_ctx_free( void *ctx )
{
    bscomptls_ripemd160_free( (bscomptls_ripemd160_context *) ctx );
    bscomptls_free( ctx );
}

static void ripemd160_clone_wrap( void *dst, const void *src )
{
    bscomptls_ripemd160_clone( (bscomptls_ripemd160_context *) dst,
                       (const bscomptls_ripemd160_context *) src );
}

static void ripemd160_process_wrap( void *ctx, const unsigned char *data )
{
    bscomptls_ripemd160_process( (bscomptls_ripemd160_context *) ctx, data );
}

const bscomptls_md_info_t bscomptls_ripemd160_info = {
    BSCOMPTLS_MD_RIPEMD160,
    "RIPEMD160",
    20,
    64,
    ripemd160_starts_wrap,
    ripemd160_update_wrap,
    ripemd160_finish_wrap,
    bscomptls_ripemd160,
    ripemd160_ctx_alloc,
    ripemd160_ctx_free,
    ripemd160_clone_wrap,
    ripemd160_process_wrap,
};

#endif /* BSCOMPTLS_RIPEMD160_C */

#if defined(BSCOMPTLS_SHA1_C)

static void sha1_starts_wrap( void *ctx )
{
    bscomptls_sha1_starts( (bscomptls_sha1_context *) ctx );
}

static void sha1_update_wrap( void *ctx, const unsigned char *input,
                              size_t ilen )
{
    bscomptls_sha1_update( (bscomptls_sha1_context *) ctx, input, ilen );
}

static void sha1_finish_wrap( void *ctx, unsigned char *output )
{
    bscomptls_sha1_finish( (bscomptls_sha1_context *) ctx, output );
}

static void *sha1_ctx_alloc( void )
{
    void *ctx = bscomptls_calloc( 1, sizeof( bscomptls_sha1_context ) );

    if( ctx != NULL )
        bscomptls_sha1_init( (bscomptls_sha1_context *) ctx );

    return( ctx );
}

static void sha1_clone_wrap( void *dst, const void *src )
{
    bscomptls_sha1_clone( (bscomptls_sha1_context *) dst,
                  (const bscomptls_sha1_context *) src );
}

static void sha1_ctx_free( void *ctx )
{
    bscomptls_sha1_free( (bscomptls_sha1_context *) ctx );
    bscomptls_free( ctx );
}

static void sha1_process_wrap( void *ctx, const unsigned char *data )
{
    bscomptls_sha1_process( (bscomptls_sha1_context *) ctx, data );
}

const bscomptls_md_info_t bscomptls_sha1_info = {
    BSCOMPTLS_MD_SHA1,
    "SHA1",
    20,
    64,
    sha1_starts_wrap,
    sha1_update_wrap,
    sha1_finish_wrap,
    bscomptls_sha1,
    sha1_ctx_alloc,
    sha1_ctx_free,
    sha1_clone_wrap,
    sha1_process_wrap,
};

#endif /* BSCOMPTLS_SHA1_C */

/*
 * Wrappers for generic message digests
 */
#if defined(BSCOMPTLS_SHA256_C)

static void sha224_starts_wrap( void *ctx )
{
    bscomptls_sha256_starts( (bscomptls_sha256_context *) ctx, 1 );
}

static void sha224_update_wrap( void *ctx, const unsigned char *input,
                                size_t ilen )
{
    bscomptls_sha256_update( (bscomptls_sha256_context *) ctx, input, ilen );
}

static void sha224_finish_wrap( void *ctx, unsigned char *output )
{
    bscomptls_sha256_finish( (bscomptls_sha256_context *) ctx, output );
}

static void sha224_wrap( const unsigned char *input, size_t ilen,
                    unsigned char *output )
{
    bscomptls_sha256( input, ilen, output, 1 );
}

static void *sha224_ctx_alloc( void )
{
    void *ctx = bscomptls_calloc( 1, sizeof( bscomptls_sha256_context ) );

    if( ctx != NULL )
        bscomptls_sha256_init( (bscomptls_sha256_context *) ctx );

    return( ctx );
}

static void sha224_ctx_free( void *ctx )
{
    bscomptls_sha256_free( (bscomptls_sha256_context *) ctx );
    bscomptls_free( ctx );
}

static void sha224_clone_wrap( void *dst, const void *src )
{
    bscomptls_sha256_clone( (bscomptls_sha256_context *) dst,
                    (const bscomptls_sha256_context *) src );
}

static void sha224_process_wrap( void *ctx, const unsigned char *data )
{
    bscomptls_sha256_process( (bscomptls_sha256_context *) ctx, data );
}

const bscomptls_md_info_t bscomptls_sha224_info = {
    BSCOMPTLS_MD_SHA224,
    "SHA224",
    28,
    64,
    sha224_starts_wrap,
    sha224_update_wrap,
    sha224_finish_wrap,
    sha224_wrap,
    sha224_ctx_alloc,
    sha224_ctx_free,
    sha224_clone_wrap,
    sha224_process_wrap,
};

static void sha256_starts_wrap( void *ctx )
{
    bscomptls_sha256_starts( (bscomptls_sha256_context *) ctx, 0 );
}

static void sha256_wrap( const unsigned char *input, size_t ilen,
                    unsigned char *output )
{
    bscomptls_sha256( input, ilen, output, 0 );
}

const bscomptls_md_info_t bscomptls_sha256_info = {
    BSCOMPTLS_MD_SHA256,
    "SHA256",
    32,
    64,
    sha256_starts_wrap,
    sha224_update_wrap,
    sha224_finish_wrap,
    sha256_wrap,
    sha224_ctx_alloc,
    sha224_ctx_free,
    sha224_clone_wrap,
    sha224_process_wrap,
};

#endif /* BSCOMPTLS_SHA256_C */

#if defined(BSCOMPTLS_SHA512_C)

static void sha384_starts_wrap( void *ctx )
{
    bscomptls_sha512_starts( (bscomptls_sha512_context *) ctx, 1 );
}

static void sha384_update_wrap( void *ctx, const unsigned char *input,
                                size_t ilen )
{
    bscomptls_sha512_update( (bscomptls_sha512_context *) ctx, input, ilen );
}

static void sha384_finish_wrap( void *ctx, unsigned char *output )
{
    bscomptls_sha512_finish( (bscomptls_sha512_context *) ctx, output );
}

static void sha384_wrap( const unsigned char *input, size_t ilen,
                    unsigned char *output )
{
    bscomptls_sha512( input, ilen, output, 1 );
}

static void *sha384_ctx_alloc( void )
{
    void *ctx = bscomptls_calloc( 1, sizeof( bscomptls_sha512_context ) );

    if( ctx != NULL )
        bscomptls_sha512_init( (bscomptls_sha512_context *) ctx );

    return( ctx );
}

static void sha384_ctx_free( void *ctx )
{
    bscomptls_sha512_free( (bscomptls_sha512_context *) ctx );
    bscomptls_free( ctx );
}

static void sha384_clone_wrap( void *dst, const void *src )
{
    bscomptls_sha512_clone( (bscomptls_sha512_context *) dst,
                    (const bscomptls_sha512_context *) src );
}

static void sha384_process_wrap( void *ctx, const unsigned char *data )
{
    bscomptls_sha512_process( (bscomptls_sha512_context *) ctx, data );
}

const bscomptls_md_info_t bscomptls_sha384_info = {
    BSCOMPTLS_MD_SHA384,
    "SHA384",
    48,
    128,
    sha384_starts_wrap,
    sha384_update_wrap,
    sha384_finish_wrap,
    sha384_wrap,
    sha384_ctx_alloc,
    sha384_ctx_free,
    sha384_clone_wrap,
    sha384_process_wrap,
};

static void sha512_starts_wrap( void *ctx )
{
    bscomptls_sha512_starts( (bscomptls_sha512_context *) ctx, 0 );
}

static void sha512_wrap( const unsigned char *input, size_t ilen,
                    unsigned char *output )
{
    bscomptls_sha512( input, ilen, output, 0 );
}

const bscomptls_md_info_t bscomptls_sha512_info = {
    BSCOMPTLS_MD_SHA512,
    "SHA512",
    64,
    128,
    sha512_starts_wrap,
    sha384_update_wrap,
    sha384_finish_wrap,
    sha512_wrap,
    sha384_ctx_alloc,
    sha384_ctx_free,
    sha384_clone_wrap,
    sha384_process_wrap,
};

#endif /* BSCOMPTLS_SHA512_C */

#endif /* BSCOMPTLS_MD_C */

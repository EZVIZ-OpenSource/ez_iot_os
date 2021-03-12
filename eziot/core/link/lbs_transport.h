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
#ifndef H_LBS_TRANSPORT_H_
#define H_LBS_TRANSPORT_H_

#define LBS_TRANSPORT_INTERFACE	 \
	extern ezdev_sdk_kernel_error lbs_redirect(ezdev_sdk_kernel* sdk_kernel); \
	extern ezdev_sdk_kernel_error lbs_redirect_with_auth(ezdev_sdk_kernel* sdk_kernel, EZDEV_SDK_UINT8 nUpper); \
	extern ezdev_sdk_kernel_error lbs_redirect_createdevid_with_auth(ezdev_sdk_kernel* sdk_kernel, EZDEV_SDK_UINT8 nUpper); \
	extern ezdev_sdk_kernel_error lbs_getstun(ezdev_sdk_kernel* sdk_kernel, stun_info* ptr_stun);\
	extern ezdev_sdk_kernel_error cnt_state_lbs_apply_serectkey(ezdev_sdk_kernel* sdk_kernel, EZDEV_SDK_UINT16 *interval, EZDEV_SDK_UINT32 *duration);
#endif //H_LBS_TRANSPORT_H_
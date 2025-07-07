/* 
 * Copyright (c) 2015-2021, Extrems' Corner.org
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <gccore.h>
#include "3ds.h"
#include "network.h"

#define LWP_PRIO_NORMAL 64

static lwp_t thread = LWP_THREAD_NULL;

network_state_t network = {
	.inited   = (-1),
	.use_dhcp = true
};

static void *thread_func(void *arg)
{
#ifdef HW_RVL //redundant
	network.inited = if_configex(&network.address, &network.gateway, &network.netmask, network.use_dhcp);

	if (network.inited < 0) {
		network.disabled = true;
		return NULL;
	}

	CTRInit();
#endif
	return NULL;
}

void NetworkInit(void)
{
	if (network.disabled) return;
	if (LWP_CreateThread(&thread, thread_func, NULL, NULL, 0, LWP_PRIO_NORMAL) < 0)
		network.disabled = true;
}

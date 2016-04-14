/*
 * cdv.c: Intel Cedarview platform specific setup code
 *
 * (C) Copyright 2012 Intel Corporation
 * Author: Jorge A. Ramirez 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 */


#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/setup.h>
#include <asm/reboot.h>
#include <asm/e820.h>

static char* __init cdv_memory_setup(void)
{
        char *who;
	u64 max_ram = 1UL<<32;
	who = default_machine_specific_memory_setup();
	e820_remove_range(max_ram, ULLONG_MAX-max_ram, E820_RAM,1);
	update_e820();
	return who;
}

void __init x86_cedarview_early_setup(void)
{
	/*
	 * CDV platform only maps 4GB of DDR.
	 */
	x86_init.resources.memory_setup = cdv_memory_setup;
}


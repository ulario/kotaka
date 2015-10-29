/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <kernel/kernel.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <status.h>

inherit SECOND_AUTO;

int frag;

static void create()
{
	call_out("check", 1);
}

static void check()
{
	float smem_size;
	float smem_used;
	float smem_free;

	float dmem_size;
	float dmem_used;
	float dmem_free;

	call_out("check", 1);

	smem_used = (float)status(ST_SMEMUSED);
	smem_size = (float)status(ST_SMEMSIZE);
	smem_free = smem_size - smem_used;

	dmem_used = (float)status(ST_DMEMUSED);
	dmem_size = (float)status(ST_DMEMSIZE);
	dmem_free = dmem_size - dmem_used;

	if (dmem_used > (float)(1 << 30)) {
		LOGD->post_message("system", LOG_NOTICE, "Memory full, swapping out");
		swapout();
		return;
	}

	if (dmem_free > (float)(4 << 20) && dmem_free / dmem_size > 0.25) {
		LOGD->post_message("system", LOG_NOTICE, "Memory fragmented, swapping out");
		swapout();
	}
}

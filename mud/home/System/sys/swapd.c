/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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

#define K (1 << 10)
#define M (1 << 20)
#define G (1 << 30)

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

	float mem_size;
	float mem_used;
	float mem_free;

	call_out("check", 1);

	smem_used = (float)status(ST_SMEMUSED);
	smem_size = (float)status(ST_SMEMSIZE);
	smem_free = smem_size - smem_used;

	dmem_used = (float)status(ST_DMEMUSED);
	dmem_size = (float)status(ST_DMEMSIZE);
	dmem_free = dmem_size - dmem_used;

	mem_used = smem_used + dmem_size;
	mem_size = smem_size + dmem_size;
	mem_free = smem_free + dmem_free;

	if (dmem_free > (float)M * 128.0 && mem_free / mem_size > 0.25) {
		LOGD->post_message("system", LOG_NOTICE, "Memory fragmented, swapping out");
		swapout();
	}
}

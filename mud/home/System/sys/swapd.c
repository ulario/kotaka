/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
inherit "~/lib/string/align";

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
	float dmem_size;
	float dmem_used;
	int swap;
	int full;

	call_out("check", 1);

	smem_size = (float)status(ST_SMEMSIZE);
	smem_used = (float)status(ST_SMEMUSED);
	dmem_size = (float)status(ST_DMEMSIZE);
	dmem_used = (float)status(ST_DMEMUSED);

	if (smem_size + dmem_size > (float)G) {
		swap = 1;
		full = 1;
	} else if ((smem_size + dmem_used) / (smem_size + dmem_size) < 0.75) {
		swap = 1;
	}

	if (swap) {
		int len;
		int sz;

		string ss, su;
		string ds, du;

		ss = "" + floor(smem_size / (float)M);
		su = "" + floor(smem_used / (float)M);
		ds = "" + floor(dmem_size / (float)M);
		du = "" + floor(dmem_used / (float)M);

		len = strlen(ds);

		sz = strlen(du);

		if (sz > len) {
			len = sz;
		}

		sz = strlen(ss);

		if (sz > len) {
			len = sz;
		}

		sz = strlen(su);

		if (sz > len) {
			len = sz;
		}

		if (full) {
			LOGD->post_message("system", LOG_NOTICE, "Memory fragmented, swapping out");
		} else {
			LOGD->post_message("system", LOG_NOTICE, "Memory full, swapping out");
		}

		LOGD->post_message("system", LOG_NOTICE, "Static memory allocated:  " + ralign(ss, len) + " MiB");
		LOGD->post_message("system", LOG_NOTICE, "Static memory used:       " + ralign(su, len) + " MiB");
		LOGD->post_message("system", LOG_NOTICE, "Dynamic memory allocated: " + ralign(ds, len) + " MiB");
		LOGD->post_message("system", LOG_NOTICE, "Dynamic memory used:      " + ralign(du, len) + " MiB");
		swapout();
	}
}

/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020, 2021  Raymond Jennings
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

#define K (1 << 10)
#define M (1 << 20)
#define G (1 << 30)

inherit SECOND_AUTO;

int force;
int frag;

private float roundsig(float v, int s)
{
	float smul;
	float imul;

	imul = 1.0;

	while (v > 1.0) {
		v /= 10.0;
		imul *= 10.0;
	}

	smul = pow(10.0, (float)s);

	v *= smul;
	v = (float)(int)v;
	v /= smul;

	v *= imul;

	return v;
}

static void create()
{
	call_out("check", 1);
}

static void check()
{
	float mem_used;
	float mem_size;

	string suffix;

	wipe_callouts();

	call_out("check", 1);

	mem_used = (float)status(ST_DMEMUSED) + (float)status(ST_SMEMUSED);
	mem_size = (float)status(ST_DMEMSIZE) + (float)status(ST_SMEMSIZE);

	if (mem_used / mem_size < 0.5) {
		frag++;
	} else if (frag) {
		frag--;
	}

	if (force || frag > 60) {
		if (mem_size > (float)G) {
			mem_size /= (float)G;
			mem_used /= (float)G;
			suffix = "GiB";
		} else if (mem_size > (float)M) {
			mem_size /= (float)M;
			mem_used /= (float)M;
			suffix = "MiB";
		} else {
			mem_size /= (float)K;
			mem_used /= (float)K;
			suffix = "KiB";
		}

		LOGD->post_message("system", LOG_NOTICE, "SwapD: Swapping out (" + (force ? "forced, " : "") + (frag ? "fragmented, only " : "") + "using " + roundsig(mem_used, 3) + suffix + " of " + roundsig(mem_size, 3) + suffix + ")");

		::swapout();

		force = 0;
		frag = 0;
	}
}

void swapout()
{
	force = 1;

	call_out("check", 0);
}

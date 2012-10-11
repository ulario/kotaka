/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <status.h>

#include <kernel/kernel.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

inherit SECOND_AUTO;

#define MAX_MEMORY	(1 << 28)	/* 256 megabytes */
#define FRAG_RATIO	(0.25)		/* one quarter free */
#define FREE_SLACK	(1 << 25)	/* 32 megabytes */

int callout;
int frag_angst;

static void create()
{
}

void enable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN() || LOCAL());

	if (callout) {
		error("Watchdog already enabled");
	}

	rlimits (0; -1) {
		callout = call_out("check", 1);
	}
}

void disable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	if (!callout) {
		error("Watchdog not enabled");
	}

	rlimits (0; -1) {
		remove_call_out(callout);
		callout = 0;
	}
}

int enabled()
{
	return !!callout;
}

void reboot()
{
	ACCESS_CHECK(SYSTEM());
}

static void check()
{
	float mem_size;
	float mem_used;
	float mem_free;

	int obj_size;
	int obj_used;
	int obj_free;

	int swap_size;
	int swap_used;
	int swap_free;

	int freeze;

	rlimits(0; -1) {
		callout = call_out("check", 1);
	}

	mem_used = (float)status(ST_DMEMUSED) + (float)status(ST_SMEMUSED);
	mem_size = (float)status(ST_DMEMSIZE) + (float)status(ST_SMEMSIZE);
	mem_free = mem_size - mem_used;

	obj_used = status(ST_NOBJECTS);
	obj_size = status(ST_OTABSIZE);
	obj_free = obj_size - obj_used;

	swap_used = status(ST_SWAPUSED);
	swap_size = status(ST_SWAPSIZE);
	swap_free = swap_size - swap_used;

#if 0
	if ((float)obj_free / (float)obj_size < 0.25) {
		if (CALLOUTD->query_suspend() != -1) {
			CALLOUTD->suspend_callouts();
		}
	}

	if ((float)swap_free / (float)swap_size < 0.25) {
		if (CALLOUTD->query_suspend() != -1) {
			CALLOUTD->suspend_callouts();
		}
	}
#endif

	if (mem_used > (float)MAX_MEMORY) {
		LOGD->post_message("watchdog", LOG_NOTICE, "Memory full, swapping out");
		swapout();
		return;
	}

	if (((mem_free - (float)FREE_SLACK) / mem_size) > (float)FRAG_RATIO) {
		if (!frag_angst) {
			LOGD->post_message("watchdog", LOG_NOTICE, "Memory fragmented");
		}

		++frag_angst;

		if (frag_angst >= 180) {
			frag_angst -= 120;
			LOGD->post_message("watchdog", LOG_NOTICE, "Memory still fragmented, swapping out");
			swapout();
		}
	} else {
		if (frag_angst) {
			if (!--frag_angst) {
				LOGD->post_message("watchdog", LOG_NOTICE, "Memory no longer fragmented");
			}
		}
	}
}

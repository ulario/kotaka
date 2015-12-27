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

#define FULL     86400 /* full dump every 24 hours */
#define INTERVAL   600 /* incremental dump every 10 minutes */
#define OFFSET       0 /* offset for each dump */

private int *delay();
private void purge_callouts();

static void create()
{
	int delay, full;

	({ delay, full }) = delay();

	call_out("dump", delay, full);
}

private void purge_callouts()
{
	int sz;
	mixed **callouts;

	callouts = status(this_object(), O_CALLOUTS);

	for (sz = sizeof(callouts) - 1; sz >= 0; --sz) {
		remove_call_out(callouts[sz][CO_HANDLE]);
	}
}

void upgrade()
{
	int delay;
	int full;

	({ delay, full }) = delay();

	purge_callouts();

	call_out("dump", delay, full);
}

void reboot()
{
	int delay;
	int full;

	({ delay, full }) = delay();

	purge_callouts();

	call_out("dump", delay, full);
}

private int *delay()
{
	int time;
	int goal;
	int delay;

	time = time();
	goal = time;
	goal -= goal % INTERVAL;
	goal += OFFSET;

	while (goal <= time) {
		goal += INTERVAL;
	}

	while (goal - time > INTERVAL) {
		goal -= INTERVAL;
	}

	delay = goal - time;

	return ({ delay, (goal - OFFSET) % FULL == 0 });
}

static void dump(int full)
{
	int delay;

	if (full) {
		LOGD->post_message("debug", LOG_DEBUG, "DumpD: Making a full snapshot");
		dump_state();
	} else {
		dump_state(1);
	}

	({ delay, full }) = delay();

	purge_callouts();

	call_out("dump", delay, full);
}

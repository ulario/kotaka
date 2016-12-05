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
#include <kotaka/paths/utility.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <status.h>

inherit SECOND_AUTO;

#define FULL     86400 /* full dump every 24 hours */
#define INTERVAL   600 /* incremental dump every 10 minutes */
#define OFFSET       0 /* offset for each dump */

private void purge_callouts();
static void reschedule();

static void create()
{
}

void boot()
{
	ACCESS_CHECK(SYSTEM());

	reschedule();
}

static void reschedule()
{
	int idelay;
	int fdelay;
	int time;

	purge_callouts();

	time = time();

	idelay = SUBD->idelay(time, INTERVAL, OFFSET);
	fdelay = SUBD->idelay(time, FULL, OFFSET);

	if (idelay == fdelay) {
		call_out("dump", idelay, 1);
	} else {
		call_out("dump", idelay, 0);
	}
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
	ACCESS_CHECK(SYSTEM());

	if (INITD->booted()) {
		/* ignore if this is boot time object discovery */
		reschedule();
	}
}

void reboot()
{
	ACCESS_CHECK(SYSTEM());

	reschedule();
}

static void dump(int full)
{
	int delay;

	if (full) {
		dump_state();
	} else {
		dump_state(1);
	}

	reschedule();
}

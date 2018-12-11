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
#include <kotaka/assert.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>

inherit SECOND_AUTO;

int interval;	/* how long between dumps */

private void stop()
{
	mixed **callouts;
	int sz;

	callouts = status(this_object(), O_CALLOUTS);

	for (sz = sizeof(callouts); --sz >= 0; ) {
		remove_call_out(callouts[sz][CO_HANDLE]);
	}
}

private void start()
{
	if (interval) {
		int now;
		int goal;
		int incr;

		now = time();

		goal = now;
		goal -= goal % interval;

		if (goal <= now) {
			goal += interval;
		}

		call_out("dump", goal - now);
	}
}

static void dump(varargs int incr)
{
	dump_state();
	start();
}

static void create()
{
}

void upgrade()
{
	ACCESS_CHECK(SYSTEM());

	stop();
	start();
}

void reboot()
{
	ACCESS_CHECK(SYSTEM());

	stop();
	start();
}

void set_interval(int new_interval)
{
	ACCESS_CHECK(SYSTEM());

	stop();

	interval = new_interval;

	start();
}

int query_interval()
{
	return interval;
}

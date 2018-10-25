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

int offset;	/* the remainder when time is divided by the interval */
int interval;	/* how long between dumps */
int increments;	/* the N in every Nth dump being a full dump */

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
		goal += offset;

		if (goal <= now) {
			goal += interval;
		}

		call_out("dump", goal - now, goal % (interval * increments) != offset);
	}
}

static void dump(int incr)
{
	if (incr) {
		dump_state(1);
		start();
	} else {
		dump_state();
		start();
	}
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

void set_parameters(int new_interval, int new_offset, int new_increments)
{
	ACCESS_CHECK(SYSTEM());

	if (new_interval) {
		ASSERT(new_offset < new_interval);
	} else {
		ASSERT(new_offset == 0 && new_increments == 0);
	}

	stop();

	interval = new_interval;
	offset = new_offset;
	increments = new_increments;

	start();
}

int query_interval()
{
	return interval;
}

int query_offset()
{
	return offset;
}

int query_increments()
{
	return increments;
}

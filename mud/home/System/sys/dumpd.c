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

#define INTERVAL 3600
#define OFFSET 0

private int delay();

static void create()
{
	call_out("dump", delay());
}

void upgrade()
{
	int sz;
	mixed **callouts;

	callouts = status(this_object(), O_CALLOUTS);

	for (sz = sizeof(callouts) - 1; sz >= 0; --sz) {
		remove_call_out(callouts[sz][CO_HANDLE]);
	}

	call_out("dump", delay());
}

private int delay()
{
	int time;
	int goal;
	int delay;

	time = time();
	goal = time;
	goal -= goal % INTERVAL;
	goal += INTERVAL;
	goal += OFFSET;

	delay = goal - time;
	delay %= INTERVAL;

	return delay;
}

static void dump()
{
	call_out("dump", delay());

	dump_state();
}

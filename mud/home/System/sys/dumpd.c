/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2019, 2020  Raymond Jennings
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
#include <kotaka/log.h>
#include <status.h>

inherit SECOND_AUTO;

int interval;
int steps;

private void wipe()
{
	int sz;
	mixed *callouts;

	LOGD->post_message("system", LOG_NOTICE, "DumpD: Wiping callouts");

	for (sz = sizeof(callouts = status(this_object(), O_CALLOUTS)); --sz >= 0; ) {
		remove_call_out(callouts[sz][CO_HANDLE]);
	}
}

private void start()
{
	int now;
	int goal;
	int delay;

	now = time();

	LOGD->post_message("system", LOG_NOTICE, "DumpD: Starting dump cycle");

	goal = now;
	goal -= goal % interval;
	goal += interval;
	delay = goal - now;

	call_out("dump", delay, goal);
}

private void configure()
{
	steps = 86400 / 10; /* ten minutes between incremental dumps */
	interval = 86400 / steps; /* one day between full dumps */
}

static void create()
{
	configure();
	start();
}

static void dump(int goal)
{
	int now;
	int delta;

	if (goal % interval == goal % (interval * steps)) {
		LOGD->post_message("system", LOG_NOTICE, "DumpD: Full statedump");
		dump_state();
	} else {
		LOGD->post_message("system", LOG_NOTICE, "DumpD: Incremental statedump");
		dump_state(1);
	}

	now = time();

	delta = now - goal;

	LOGD->post_message("system", LOG_NOTICE, "DumpD: timestamp delta is " + delta);

	if (delta > interval) {
		LOGD->post_message("system", LOG_NOTICE, "DumpD: schedule ruined, restarting");
		wipe();
		start();
	} else {
		goal += interval;
		delta = goal - now;

		LOGD->post_message("system", LOG_NOTICE, "DumpD: scheduling next dump, due in " + delta + " seconds");

		wipe();
		call_out("dump", delta, goal);
	}
}

void upgrade()
{
	wipe();
	configure();
	start();
}

void reboot()
{
	wipe();
	configure();
	start();
}

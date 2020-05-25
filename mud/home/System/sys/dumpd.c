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

#define FULL 600
#define INCR 600

inherit SECOND_AUTO;

int interval;
int steps;
int offset;

private void start()
{
	int now;
	int goal;
	int delay;

	if (interval <= 0) {
		return;
	}

	now = time();

	goal = now;
	goal -= goal % interval;
	goal += offset;
	goal += interval;
	delay = goal - now;
	delay %= interval;
	goal = now + delay;

	wipe_callouts();
	call_out("dump", delay, goal);
}

private void configure()
{
	steps = FULL / INCR; /* ten minutes between incremental dumps */
	interval = FULL / steps; /* one day between full dumps */
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

	if (delta) {
		LOGD->post_message("system", LOG_NOTICE, "DumpD: Dump callout executed " + delta + " seconds late");

		if (delta > interval) {
			LOGD->post_message("system", LOG_NOTICE, "DumpD: Stall exceeds interval, restarting cycle");
			start();
			return;
		}
	}

	goal += interval;
	delta = goal - now;

	call_out("dump", delta, goal);
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	LOGD->post_message("system", LOG_NOTICE, "DumpD: Recompiled, restarting dump cycle");

	configure();
	start();
}

void reboot()
{
	ACCESS_CHECK(previous_program() == INITD);

	LOGD->post_message("system", LOG_NOTICE, "DumpD: Rebooted, restarting dump cycle");

	configure();
	start();
}

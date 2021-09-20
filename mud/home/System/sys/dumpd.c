/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2019, 2020, 2021  Raymond Jennings
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

#define INTERVAL    600
#define OFFSET        0
#define INCREMENTS    6

inherit SECOND_AUTO;

int increments;

private void start(varargs string restart)
{
	int now;
	float fnow;

	int goal;

	int delay;
	float fdelay;

	if (INTERVAL <= 0) {
		return;
	}

	({ now, fnow }) = millitime();

	goal = now;
	goal -= goal % INTERVAL;
	goal += OFFSET;
	goal += INTERVAL;
	delay = goal - now;
	delay %= INTERVAL;
	goal = now + delay;

	fdelay = (float)delay - fnow;

	if (fdelay < 0.5) {
		fdelay = 0.5;
	}

	if (restart) {
		LOGD->post_message("system", LOG_NOTICE, "DumpD: Restarting snapshot cycle: " + restart);
	}

	wipe_callouts();
	call_out("dump", fdelay, goal);
}

private void report_config()
{
	LOGD->post_message("system", LOG_NOTICE, "DumpD: Snapshot interval: " + INTERVAL);
	LOGD->post_message("system", LOG_NOTICE, "DumpD: Snapshot offset: " + OFFSET);
	LOGD->post_message("system", LOG_NOTICE, "DumpD: Snapshot increments: " + INCREMENTS);
}

static void create()
{
	report_config();
	start();
}

static void dump(int goal)
{
	int now;
	float fnow;

	int delta;
	float fdelta;

	increments++;

	if (increments >= INCREMENTS) {
		dump_state();
		increments = 0;
	} else {
		dump_state(1);
	}

	({ now, fnow }) = millitime();

	delta = now - goal;
	fdelta = (float)delta + fnow;

	if (fdelta >= 1.0) {
		LOGD->post_message("system", LOG_NOTICE, "DumpD: Dump callout executed " + fdelta + " seconds late");

		if (delta > INTERVAL) {
			start("lagged");
			return;
		}
	}

	goal += INTERVAL;
	delta = goal - now;
	fdelta = (float)delta - fnow;

	if (fdelta < 0.5) {
		fdelta = 0.5;
	}

	wipe_callouts();

	call_out("dump", fdelta, goal);
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	report_config();
	start("recompiled");
}

void reboot()
{
	ACCESS_CHECK(previous_program() == INITD);

	report_config();
	start("rebooted");
}

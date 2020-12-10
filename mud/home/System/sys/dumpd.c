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

inherit SECOND_AUTO;

int interval;
int offset;

private void start()
{
	int now;
	float fnow;

	int goal;

	int delay;
	float fdelay;

	if (interval <= 0) {
		return;
	}

	({ now, fnow }) = millitime();

	goal = now;
	goal -= goal % interval;
	goal += offset;
	goal += interval;
	delay = goal - now;
	delay %= interval;
	goal = now + delay;

	fdelay = (float)delay - fnow;

	if (fdelay < 0.5) {
		fdelay = 0.5;
	}

	wipe_callouts();
	call_out("dump", fdelay, goal);
}

private void configure()
{
	interval = FULL;
}

static void create()
{
	configure();
	start();
}

static void dump(int goal)
{
	int now;
	float fnow;

	int delta;
	float fdelta;

	dump_state();

	({ now, fnow }) = millitime();

	delta = now - goal;
	fdelta = (float)delta + fnow;

	if (fdelta >= 1.0) {
		LOGD->post_message("system", LOG_NOTICE, "DumpD: Dump callout executed " + delta + " seconds late");

		if (delta > interval) {
			LOGD->post_message("system", LOG_NOTICE, "DumpD: Stall exceeds interval, restarting cycle");
			start();
			return;
		}
	}

	goal += interval;
	delta = goal - now;
	fdelta = (float)delta - fnow;

	if (fdelta < 0.5) {
		fdelta = 0.5;
	}

	call_out("dump", fdelta, goal);
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

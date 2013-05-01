/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012-2013  Raymond Jennings
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
#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <status.h>
#include <thing/paths.h>

inherit "~Thing/lib/thing";

private float interval;
private int handle;

void clear_timer()
{
	if (handle) {
		remove_call_out(handle);
		handle = 0;
	}
}

void set_timer(float delay, float new_interval)
{
	if (handle) {
		remove_call_out(handle);
		handle = 0;
	}

	interval = new_interval;

	handle = call_out("tick", delay);
}

private int overloaded()
{
	mixed **co;
	int sz, i;

	co = status(this_object(), O_CALLOUTS);
	sz = sizeof(co);

	for (i = 0; i < sz; i++) {
		if (co[i][CO_FUNCTION] == "tick") {
			return 1;
		}
	}

	return 0;
}

static void tick()
{
	string handler;

	handle = 0;

	if (overloaded()) {
		LOGD->post_message("timer", LOG_DEBUG, "Too many timer callouts, dropping a tick");
		return;
	}

	handler = query_property("event:timer");

	if (handler) {
		handler->on_timer(this_object());
	} else {
		error("No handler");
	}

	if (handle) {
		/* changed/restarted in handler */
		return;
	}

	if (this_object() && interval >= 0.0 && !handle) {
		handle = call_out("tick", interval);
	}
}

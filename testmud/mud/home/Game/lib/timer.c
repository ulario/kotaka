/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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

inherit LIB_OBJECT;

float min_interval;
float max_interval;

int handle;

void clear_timer()
{
	if (handle) {
		remove_call_out(handle);
		handle = 0;
	}
}

void set_timer(float delay, float low, float high)
{
	if (high < low) {
		error("Illegal interval");
	}

	min_interval = low;
	max_interval = high;

	if (handle) {
		remove_call_out(handle);
	}

	call_out("tick", delay);
}

static void tick()
{
	string handler;

	handler = query_property("timer");

	if (handler) {
		handler->on_timer(this_object());
	} else {
		error("No handler");
	}

	if (this_object() && max_interval > 0.0) {
		call_out("tick", min_interval + SUBD->rnd() * (max_interval - min_interval));
	}
}

/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2021  Raymond Jennings
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
#include <kotaka/assert.h>
#include <status.h>
#include <type.h>

inherit "/lib/string/sprint";

private mapping handles;
private mapping intervals;

mixed query_property(string property);

atomic private void timer_arm(string name, float delay)
{
	int handle;

	handle = call_out("timer_expired", delay, name);

	if (!handles) {
		handles = ([ ]);
	}

	handles[name] = handle;
}

atomic private float timer_disarm(string name)
{
	int handle;

	if (handles && (handle = handles[name])) {
		float delay;

		delay = remove_call_out(handle);
		handles[name] = 0;

		if (map_sizeof(handles) == 0) {
			handles = nil;
		}

		return delay;
	}

	return -1.0;
}

static void timer_expired(string name)
{
	string handler;
	float interval;

	ASSERT(name);
	ASSERT(typeof(handles) == T_MAPPING);

	/* clear the handle immediately after the callout expires */
	handles[name] = nil;

	if (!map_sizeof(handles)) {
		handles = nil;
	}

	handler = query_property("timer:" + name);

	if (!handler) {
		/* no handler, shut off the timer */
		return;
	}

	handler->on_timer(this_object(), name);

	if (!intervals) {
		return;
	}

	interval = intervals[name];

	if (interval > 0.0) {
		timer_arm(name, interval);
	}
}

static mapping timer_save()
{
	mapping delays;

	if (handles) {
		int sz;
		mixed **callouts;
		string *names;

		delays = ([ ]);

		callouts = status(this_object(), O_CALLOUTS);

		for (sz = sizeof(callouts); --sz >= 0; ) {
			mixed *callout;

			callout = callouts[sz];

			delays[callout[CO_FIRSTXARG]] = callout[CO_DELAY];
		}
	}

	return ([
		"timer_delays" : delays,
		"timer_intervals" : intervals
	]);
}

static void timer_load(mapping map)
{
	mapping delays;
	string *names;
	int sz;

	intervals = map["timer_intervals"];

	/* remove old timer callouts */
	if (handles) {
		int *remove;

		remove = map_values(handles);

		for (sz = sizeof(remove); --sz >= 0; ) {
			remove_call_out(remove[sz]);
		}

		handles = nil;
	}

	delays = map["timer_delays"];

	if (!delays) {
		return;
	}

	names = map_indices(delays);

	for (sz = sizeof(names); --sz >= 0; ) {
		string name;

		name = names[sz];

		timer_arm(name, delays[name]);
	}
}

float stop_timer(string name)
{
	if (!name) {
		error("Invalid name");
	}

	return timer_disarm(name);
}

void start_timer(string name, float delay)
{
	int handle;

	if (!name) {
		error("Invalid name");
	}

	if (delay < 0.0) {
		error("Invalid delay");
	}

	if (handles && handles[name]) {
		timer_disarm(name);
	}

	timer_arm(name, delay);
}

void set_timer_interval(string name, float new_interval)
{
	if (new_interval < 0.0) {
		error("Invalid interval");
	}

	if (!name) {
		error("Invalid name");
	}

	if (!intervals) {
		intervals = ([ ]);
	}

	intervals[name] = new_interval;
}

float query_timer_delay(string name)
{
	mixed **callouts;
	int sz;
	int handle;

	if (!name) {
		error("Invalid name");
	}

	if (!handles) {
		return -1.0;
	}

	handle = handles[name];

	callouts = status(this_object(), O_CALLOUTS);

	for (sz = sizeof(callouts); --sz >= 0; ) {
		mixed *callout;

		callout = callouts[sz];

		if (callout[CO_HANDLE] == handle) {
			return callout[CO_DELAY];
		}
	}

	/* uh oh, we should have found it */
	ASSERT(0);
}

float query_timer_interval(string name)
{
	if (!name) {
		error("Invalid name");
	}

	if (!intervals) {
		return 0.0;
	}

	return intervals[name];
}

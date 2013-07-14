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
#include <kotaka/privilege.h>

#include <text/paths.h>
#include <game/paths.h>
#include <status.h>

inherit TEXT_LIB_USTATE;

int stopped;
int reading;
int introed;

mixed *oldtime;
int tsec;
int tframe;
int framerate;
int callout;

static void create()
{
	::create();
}

static void destruct()
{
	::destruct();
}

static void do_frame(float diff);

private void reset_frame_info()
{
	oldtime = millitime();

	tsec = oldtime[0];
	tframe = (int)floor(oldtime[1] * (float)framerate);
}

void begin()
{
	query_user()->set_mode(MODE_NOECHO);

	framerate = 30;

	reset_frame_info();

	ACCESS_CHECK(previous_object() == query_user());

	callout = call_out("frame", 0);
}

void stop()
{
	ACCESS_CHECK(previous_object() == query_user());

	query_user()->set_mode(MODE_ECHO);
	remove_call_out(callout);

	stopped = 1;
}

void go()
{
	ACCESS_CHECK(previous_object() == query_user());

	query_user()->set_mode(MODE_NOECHO);

	callout = call_out("frame", 0);

	reset_frame_info();

	stopped = 0;
}

static void frame()
{
	string buffer;
	object paint;
	mixed *time;
	float diff;
	mixed *frametime;

	time = millitime();

	tframe++;
	while (tframe >= framerate) {
		tframe -= framerate;
		tsec++;
	}

	frametime = ({ tsec, (float)tframe / (float)framerate });

	if (frametime[0] < time[0] || (frametime[0] == time[0] && frametime[1] < time[1])) {
		tsec = time[0];
		tframe = (int)floor(time[1] * (float)framerate);
		call_out("frame", 0);
		frametime = nil;
		diff = 0.0;
	} else {
		diff = frametime[1] - time[1];
		diff += (float)(frametime[0] - time[0]);

		call_out("frame", diff);
	}

	diff = time[1] - oldtime[1];
	diff += (float)(time[0] - oldtime[0]);
	oldtime = time;

	do_frame(diff);
}

void end()
{
	ACCESS_CHECK(previous_object() == query_user());

	query_user()->set_mode(MODE_ECHO);

	remove_call_out(callout);

	if (!sscanf(object_name(this_object()), "%*s#-1")) {
		destruct_object(this_object());
	}
}

void receive_in(string input)
{
	int test;
	string first;
	float target;

	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	if (!sscanf(input, "%s %s", first, input)) {
		first = input;
		input = "";
	}

	switch(first) {
	case "frame":
		if (!sscanf(input, "%d", test)) {
			send_out("Invalid number.\n");
			break;
		}

		if (test <= 0) {
			send_out("Illegal number.\n");
			break;
		}

		target = (float)tframe / (float)framerate;
		framerate = test;
		tframe = (int)(target * (float)framerate);

		break;

	case "quit":
		send_out("\n");
		pop_state();
		return;

	default:
		send_out(first + ": command not recognized.\n");
	}

	reading = 0;
}

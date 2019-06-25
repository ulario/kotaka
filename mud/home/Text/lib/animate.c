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
#include <kernel/user.h>
#include <kotaka/paths/text.h>
#include <kotaka/privilege.h>

inherit TEXT_LIB_USTATE;

int stopped;
int reading;
int introed;

mixed *oldtime;
int tsec;
int tframe;
int framerate;
int callout;

int screen_width;
int screen_height;

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

	ACCESS_CHECK(previous_object() == query_user());
}

void stop()
{
	ACCESS_CHECK(previous_object() == query_user());

	query_user()->set_mode(MODE_ECHO);
	remove_call_out(callout);

	send_out("\033[1;1H\033[2J");

	stopped = 1;
}

void go()
{
	ACCESS_CHECK(previous_object() == query_user());

	query_user()->set_mode(MODE_NOECHO);

	callout = call_out("frame", 0);

	reset_frame_info();

	send_out("\033[1;1H\033[2J");

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
		frametime = nil;
		diff = 0.0;
	} else {
		diff = frametime[1] - time[1];
		diff += (float)(frametime[0] - time[0]);
	}

	callout = call_out("frame", diff);

	diff = time[1] - oldtime[1];
	diff += (float)(time[0] - oldtime[0]);
	oldtime = time;

	do_frame(diff);
}

void pre_end()
{
	query_user()->set_mode(MODE_ECHO);
}

void end()
{
	ACCESS_CHECK(previous_object() == query_user());

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

static object telnet_obj()
{
	object conn;

	conn = query_user();

	while (conn && conn <- LIB_USER) {
		if (conn <- "~Text/obj/filter/telnet") {
			return conn;
		}
		conn = conn->query_conn();
	}
}

static void check_screen()
{
	object telnet;

	telnet = telnet_obj();

	if (telnet && telnet->query_naws_active()) {
		screen_width = telnet->query_naws_width();
		screen_height = telnet->query_naws_height();
		return;
	}

	screen_width = 40;
	screen_height = 10;
}

int forbid_log_outbound()
{
	return 1;
}

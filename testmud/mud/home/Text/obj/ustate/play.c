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
#include <kotaka/constant.h>
#include <kotaka/assert.h>

#include <game/paths.h>
#include <text/paths.h>

inherit TEXT_LIB_USTATE;

int stopped;
int reading;
int introed;
int facing;

object world;
object body;

static void create(int clone)
{
	::create();
	world = GAME_INITD->query_world();
}

static void destruct(int clone)
{
	::destruct();
}

private void prompt()
{
	if (body) {
		switch(body->query_property("id")) {
		case "deer":
			send_out("(\033[1;37mwhite-tailed \033[0;33mdeer\033[0m) > ");
			break;
		case "wolf":
			send_out("(\033[1;30mgray wolf\033[0m) > ");
			break;
		case "rock":
			send_out("(boring stone) > ");
			break;
		default:
			send_out(body->query_property("id") + "> ");
		}
	} else {
		send_out("(no character) > ");
	}
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	prompt();
	reading = 1;
}

void stop()
{
	ACCESS_CHECK(previous_object() == query_user());

	stopped = 1;
}

void go()
{
	ACCESS_CHECK(previous_object() == query_user());

	stopped = 0;

	if (!reading) {
		prompt();
	}
}

static void self_destruct()
{
	destruct_object(this_object());
}

private void do_help()
{
	send_out(read_file("~/data/doc/guest_help"));
}

void receive_in(string input)
{
	string first, rest;

	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	if (!sscanf(input, "%s %s", first, rest)) {
		first = input;
		rest = "";
	}

	switch(first) {
	case "quit":
		pop_state();
		return;
	case "inhabit":
		if (body) {
			send_out("You already have a body.\n");
		} else {
			object *inv;
			int sz;

			inv = world->query_inventory();
			sz = sizeof(inv);

			if (!sz) {
				send_out("There are no objects for you to inhabit.\n");
			} else {
				body = inv[random(sz)];

				switch(body->query_property("id")) {
				case "wolf":
					send_out("You are now a shaggy furred wolf with sharp teeth.\n");
					break;
				case "deer":
					send_out("You are now a sleek haired deer.\n");
					break;
				case "rock":
					send_out("You are now a granite rock.\n");
					break;
				}
			}
		}
		break;
	case "uninhabit":
		if (!body) {
			send_out("You do not presently have a body.\n");
		} else {
			body = nil;
			send_out("You step out of the object.\n");
		}
		break;
	case "face":
		if (sscanf(rest, "%d", facing)) {
			send_out("You are now facing " + facing + " degrees clockwise from due north.\n");
		} else {
			send_out("I need a number for that.\n");
		}
		break;
	case "forward":
		{
			float distance;

			if (sscanf(rest, "%f", distance)) {
				float sin, cos;

				sin = sin((float)facing * SUBD->pi() / 180.0);
				cos = cos((float)facing * SUBD->pi() / 180.0);

				body->set_x_position(body->query_x_position() + sin * distance);
				body->set_y_position(body->query_y_position() - cos * distance);

				send_out("You proceed " + distance + " meters forward.\n");
			}
		}
		break;
	default:
		if (!VERBD->do_action(first, body, input)) {
			if (!execute_command(first, input)) {
				send_out("Not a valid command or verb.\n");
			}
		}
		break;
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}

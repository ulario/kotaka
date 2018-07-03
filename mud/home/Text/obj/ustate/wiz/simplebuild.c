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
#include <game/paths.h>
#include <kotaka/assert.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>
#include <kotaka/privilege.h>

inherit TEXT_LIB_USTATE;

int reading;
int stopped;
int dead;

object body;

object roomarch;
object exitarch;
string roompainter;
string exitpainter;

string task;	/* makeroom, linkrooms */
string stage;

int north;
int south;
int east;
int west;
string id;
object room;
string wall;

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
}

private void prompt()
{
	if (task) {
		switch(task) {
		case "makeroom":
			switch(stage) {
			case "id":
				send_out("Please choose an id for the room: ");
				break;
			case "north":
				send_out("How many meters north does it extend? ");
				break;
			case "south":
				send_out("How many meters south does it extend? ");
				break;
			case "east":
				send_out("How many meters east does it extend? ");
				break;
			case "west":
				send_out("How many meters west does it extend? ");
				break;
			default:
				send_out("EEP-makeroom\n");
			}
			break;

		case "linkroom":
			switch(stage) {
			case "id":
				send_out("Please input the id of the room you want to link from: ");
				break;
			case "wall":
				send_out("Which wall would you like the exit to be in? ");
				break;
			case "offset":
				switch(wall) {
				case "north":
				case "south":
					send_out("How far east of the northwest corner do you want the exit? ");
					break;
				case "east":
				case "west":
					send_out("How far south of the northwest corner do you want the exit? ");
					break;
				}
			}
			break;

		default:
			send_out("OOPS\n");
			pop_state();
			return;
		}
	} else {
		send_out("[\033[1;33msimple builder\033[0m] ");
	}
}

void set_body(object o)
{
	ACCESS_CHECK(VERB());

	body = o;
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());
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

void pre_end()
{
	ACCESS_CHECK(previous_object() == query_user());

	dead = 1;
}

private atomic void do_makeroom()
{
	int bx, by, sx, sy;
	int px, py;

	object env;
	object creator;

	bx = body->query_property("pos_x");
	by = body->query_property("pos_y");

	px = bx - west;
	py = by - north;

	sx = west + east + 1;
	sy = north + south + 1;

	env = body->query_environment();
	ASSERT(env);

	room = GAME_INITD->create_thing();

	if (roomarch) {
		room->set_archetypes( ({ roomarch }) );
	}

	if (roompainter) {
		room->set_local_property("event:paint", roompainter);
	}

	if (id) {
		room->set_id(id);
	}

	room->move(env);
	room->set_x_position(px);
	room->set_y_position(py);
	room->set_x_size(sx);
	room->set_y_size(sy);
	room->set_capacity((float)(sx * sy * 3));
	room->set_max_mass((float)(sx * sy * 1000));

	creator = new_object("objcreate");
	creator->set_object(room);

	room = nil;

	push_state(creator);
}

private atomic void do_linkroom(int offset)
{
	object room2;
	object env;
	object *inv;
	object exit, exit2;

	int px, py;
	int sx, sy;
	int i;

	int ex, ey;
	int px2, py2;

	if (!room) {
		send_out("Odd, the room you picked seems to have disappeared.\n");
		return;
	}

	sx = room->query_x_size();
	sy = room->query_y_size();

	switch(wall) {
	case "north":
		px = offset;
		py = -1;
		break;
	case "south":
		px = offset;
		py = sy;
		break;
	case "east":
		px = sx;
		py = offset;
		break;
	case "west":
		px = -1;
		py = offset;
		break;
	}

	ex = px + room->query_x_position();
	ey = py + room->query_y_position();

	env = body->query_environment();
	inv = env->query_inventory();

	/* find the other room */
	for (i = sizeof(inv); --i >= 0; ) {
		object tmp;

		tmp = inv[i];

		px2 = tmp->query_x_position();
		py2 = tmp->query_y_position();

		if (ex < px2) {
			continue;
		}
		if (ex > px2 + tmp->query_x_size() - 1) {
			continue;
		}
		if (ey < py2) {
			continue;
		}
		if (ey > py2 + tmp->query_y_size() - 1) {
			continue;
		}
		room2 = tmp;
		break;
	}

	if (!room2) {
		send_out("There is no room on the other side of the wall.\n");
		return;
	}

	exit = GAME_INITD->create_thing();
	exit2 = GAME_INITD->create_thing();

	exit->set_archetypes( ({ exitarch }) );
	exit2->set_archetypes( ({ exitarch }) );

	exit->set_local_property("event:paint", exitpainter);
	exit2->set_local_property("event:paint", exitpainter);

	exit->set_local_property("exit_direction", wall);
	exit->set_exit_return(exit2);

	exit->move(room);
	exit->set_x_position(px);
	exit->set_y_position(py);

	exit2->move(room2);

	switch(wall) {
	case "north":
		exit2->set_local_property("exit_direction", "south");
		exit2->set_x_position(ex - px2);
		exit2->set_y_position(ey - py2 + 1);
		break;
	case "south":
		exit2->set_local_property("exit_direction", "north");
		exit2->set_x_position(ex - px2);
		exit2->set_y_position(ey - py2 - 1);
		break;
	case "east":
		exit2->set_local_property("exit_direction", "west");
		exit2->set_x_position(ex - px2 - 1);
		exit2->set_y_position(ey - py2);
		break;
	case "west":
		exit2->set_local_property("exit_direction", "east");
		exit2->set_x_position(ex - px2 + 1);
		exit2->set_y_position(ey - py2);
		break;
	}
}

private void do_input(string input)
{
	mixed tmp;

	if (task) {
		switch(task) {
		case "makeroom":
			switch(stage) {
			case "id":
				if (input != "") {
					id = input;
					stage = "north";
				} else {
					id = nil;
					stage = "north";
				}
				break;
			case "north":
			case "south":
			case "east":
			case "west":
				if (sscanf(input, "%d", tmp)) {
					switch(stage) {
					case "north":
						north = tmp;
						stage = "south";
						break;
					case "south":
						south = tmp;
						stage = "east";
						break;
					case "east":
						east = tmp;
						stage = "west";
						break;
					case "west":
						west = tmp;
						do_makeroom();
						task = nil;
						break;
					}
				} else {
					send_out("That doesn't look like an integer.  Try again.\n");
					break;
				}
			}
			break;

		case "linkroom":
			switch(stage) {
			case "id":
				if (room = body->query_environment()
					->find_by_id(input)) {
					stage = "wall";
				} else {
					send_out("No such room.  Try again.\n");
				}
				break;

			case "wall":
				switch(input) {
				case "north":
				case "south":
				case "east":
				case "west":
					wall = input;
					stage = "offset";
					break;
				}
				break;

			case "offset":
				if (sscanf(input, "%d", tmp)) {
					do_linkroom(tmp);
					task = nil;
					break;
				} else {
					send_out("That doesn't look like an integer.  Try again.\n");
				}
			}
			break;
		}
	} else {
		string args;
		string first;

		if (!sscanf(input, "%s %s", first, args)) {
			first = input;
			args = "";
		}

		switch(first) {
		case "":
			break;

		case "help":
			{
				send_out("Commands:\n\n");
				send_out("help           - this help message.\n");
				send_out("walk           - walk around.\n");
				send_out("look           - look around.\n");
				send_out("quit           - Quit the simplebuilder.\n");
				send_out("setroomarch    - Set the archetype for new rooms.\n");
				send_out("setexitarch    - Set the archetype for new exits.\n");
				send_out("setroompainter - Set the painter for new rooms.\n");
				send_out("setexitpainter - Set the painter for new exits.\n");
				send_out("makeroom       - Create a new room.\n");
				send_out("linkroom       - Connect two rooms.\n");
				break;
			}
		case "walk":
		case "look":
			{
				object verb;

				verb = VERBD->find_verb(first);

				if (!verb) {
					send_out("No such command.\n");
					break;
				}

				"~/sys/englishd"->do_verb(verb, first, args);
				break;
			}

		case "quit":
			stopped = 1;
			pop_state();
			return;

		case "setroomarch":
			if (!args) {
				roomarch = nil;
			} else if (!(tmp = find_object(args))
				&& !(tmp = CATALOGD->lookup_object(args))) {
			} else {
				roomarch = tmp;
			}
			break;

		case "setexitarch":
			if (!args) {
				exitarch = nil;
			} else if (!(tmp = find_object(args))
				&& !(tmp = CATALOGD->lookup_object(args))) {
				send_out("That object does not exist.\n");
			} else {
				exitarch = tmp;
			}
			break;

		case "setroompainter":
			if (!args) {
				roompainter = nil;
			} else if (!find_object(args)) {
				send_out("That object does not exist.\n");
				break;
			}

			roompainter = args;
			break;

		case "setexitpainter":
			if (!args) {
				exitpainter = nil;
			} else if (!find_object(args)) {
				send_out("That object does not exist.\n");
				break;
			}

			exitpainter = args;
			break;

		case "makeroom":
			task = "makeroom";
			stage = "id";
			break;

		case "linkroom":
			task = "linkroom";
			stage = "id";
			break;

		default:
			send_out("No such command.\n");
		}
	}
}

void receive_in(string input)
{
	ACCESS_CHECK(previous_object() == query_user());

	if (!body) {
		send_out("Your body seems to have disappeared.\n");
		pop_state();
		return;
	}

	if (!body->query_environment()) {
		send_out("Your environment seems to have disappeared.\n");
		pop_state();
		return;
	}

	reading = 1;

	catch {
		do_input(input);
	} : {
		send_out("Error.\n");
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}

void end()
{
	destruct_object(this_object());
}

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
#include <account/paths.h>
#include <game/paths.h>
#include <text/paths.h>

#define SHUFFLED "~Test/sys/shuffled"

inherit LIB_BIN;

void main(string args)
{
	string first;

	if (!sscanf(args, "%s %s", first, args)) {
		first = args;
		args = nil;
	}

	switch(first) {
	case "":
		send_out("Shuffle test commands:\n");
		send_out("stats: show statistics\n");
		send_out("rooms: set rooms goal\n");
		send_out("mobs: set mobs goal\n");
		break;
	case "stats":
		send_out("Room count/goal: " + SHUFFLED->query_room_count() + "/" + SHUFFLED->query_room_goal() + "\n");
		send_out("Mob count/goal: " + SHUFFLED->query_mob_count() + "/" + SHUFFLED->query_mob_goal() + "\n");
		break;
	case "rooms":
		{
			int number;

			if (sscanf(args, "%d", number)) {
				SHUFFLED->set_room_goal(number);
			} else {
				send_out("That's not a number");
			}
		}
		break;
	case "mobs":
		{
			int number;

			if (sscanf(args, "%d", number)) {
				SHUFFLED->set_mob_goal(number);
			} else {
				send_out("That's not a number");
			}
		}
		break;
	}
}

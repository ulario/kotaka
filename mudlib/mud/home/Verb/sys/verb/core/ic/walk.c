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
#include <kotaka/paths/string.h>
#include <kotaka/paths/verb.h>

inherit LIB_EMIT;
inherit LIB_RAWVERB;

void main(object actor, string args)
{
	int number;
	int px, py;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	args = STRINGD->to_lower(args);
	px = actor->query_property("pos_x");
	py = actor->query_property("pos_y");

	while (strlen(args)) {
		switch(args[0]) {
		case '0' .. '9':
			number *= 10;
			number += args[0] - '0';
			break;

		case 'n':
			if (number) {
				py -= number;
			} else {
				py--;
			}
			number = 0;
			break;

		case 'e':
			if (number) {
				px += number;
			} else {
				px++;
			}
			number = 0;
			break;

		case 's':
			if (number) {
				py += number;
			} else {
				py++;
			}
			number = 0;
			break;

		case 'w':
			if (number) {
				px -= number;
			} else {
				px--;
			}
			number = 0;
			break;
		
		default:
			send_out("Invalid walk string: " + args + "\n");
			args = "";
			continue;
		}

		args = args[1 ..];
	}

	actor->set_property("pos_x", px);
	actor->set_property("pos_y", py);
}

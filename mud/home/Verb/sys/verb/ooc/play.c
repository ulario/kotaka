/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020  Raymond Jennings
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
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Play";
}

string *query_help_contents()
{
	return ({ "Takes a character and puts them in the game world." });
}

void main(object actor, mapping roles)
{
	object user;
	string name;

	object ghost;

	user = query_user();
	name = user->query_name();

	if (!(name = user->query_name())) {
		send_out("You aren't logged in.\n");
		return;
	}

	ghost = IDD->find_object_by_name("ghosts:" + name);

	if (!ghost) {
		if (IDD->find_object_by_name("template:" + name)) {
			send_out("Alas, your soul is gone.\n");
		} else {
			send_out("Run chargen, you don't have a character.\n");
		}
		return;
	}

	send_out("Inhabiting " + TEXT_SUBD->generate_brief_definite(ghost) + ".\n");
	user->set_body(ghost);
}

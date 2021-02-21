/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020, 2021  Raymond Jennings
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
inherit "~Text/lib/sub";

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
	object body, ghost, newbody, user;
	string args, name;

	user = query_user();
	name = user->query_name();

	if (!name) {
		send_out("You aren't logged in.\n");
		return;
	}

	args = roles["raw"];

	if (args) {
		if (user->query_class() < 2) {
			send_out("Only wizards can play characters other than their own.\n");
			return;
		}

		ghost = IDD->find_object_by_name(args);

		if (!ghost) {
			send_out("No such thing found.\n");
			return;
		}
	} else {
		ghost = IDD->find_object_by_name("ghosts:" + name);

		if (!ghost) {
			if (IDD->find_object_by_name("template:" + name)) {
				send_out("Alas, your soul is gone.\n");
			} else {
				send_out("Run chargen, you don't have a character.\n");
			}
			return;
		}
	}

	body = ghost;

	while (newbody = body->query_possessee()) {
		body = newbody;
	}

	if (ghost->query_possessor() && user->query_class() >= 2) {
		send_out("Warning: you are playing a possessed object.\n");
	}

	send_out("Inhabiting " + generate_brief_definite(body) + ".\n");
	user->set_body(ghost);
	"~Action/sys/action/login"->action( ([ "actor": body ]) );
}

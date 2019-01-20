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
#include <kotaka/paths/utility.h>
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
	string name;
	object world;
	object body;
	object user;
	object oldbody;
	object *inv;
	object *mobiles;

	user = query_user();

	name = user->query_name();

	if (!(name = user->query_name())) {
		send_out("You aren't logged in.\n");
		return;
	}

	if (roles["raw"] != "") {
		if (user->query_class() < 2) {
			send_out("Only a wizard can play someone other than their default character.\n");
			return;
		}

		body = IDD->find_object_by_name(roles["raw"]);

		if (!body) {
			send_out("No such body found\n");
		}
	}

	body = IDD->find_object_by_name("players:" + name);

	if (!body) {
		send_out("You don't have a character.  Please use chargen to create one.\n");
		return;
	}

	if (oldbody = user->query_body()) {
		send_out("Departing " + TEXT_SUBD->generate_brief_definite(oldbody) + ".\n");
	}

	send_out("Inhabiting " + TEXT_SUBD->generate_brief_definite(body) + ".\n");

	user->set_body(body);
}

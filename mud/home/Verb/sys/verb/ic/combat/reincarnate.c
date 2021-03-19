/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2020  Raymond Jennings
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
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>
#include <kotaka/paths/system.h>
#include <game/paths.h>

inherit "/lib/string/case";
inherit LIB_EMIT;
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

/*
mixed **query_roles()
{
	return ({
		({ "iob", ({ "to" }), 0 })
	});
}
*/

string query_help_title()
{
	return "Reincarnate";
}

string *query_help_contents()
{
	return ({
		"Recreates a new body.  Only works if you don't already have one."
	});
}

void main(object actor, mapping roles)
{
	object user;
	string name;

	object template;
	object ghost;
	object body;
	object world;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	user = query_user();
	name = user->query_name();

	template = IDD->find_object_by_name("templates:" + name);

	if (!template) {
		send_out("You don't have a character, please run chargen.\n");
		return;
	}

	ghost = IDD->find_object_by_name("ghosts:" + name);

	if (!ghost) {
		send_out("Sadness...your soul has been consigned to oblivion.\n");
		return;
	}

	if (ghost->query_character_lwo()) {
		send_out("Odd...your spirit was marked as a character...resetting\n");
		ghost->clear_character();
	}

	if (ghost->query_living_lwo()) {
		send_out("Odd...your spirit was marked as living...resetting\n");
		ghost->clear_living();
	}

	body = IDD->find_object_by_name("players:" + name);

	if (body) {
		send_out("You already have a body.\n");

		if (body->query_living_lwo()) {
			send_out("And it seems to be alive already.\n");
		} else {
			send_out("You're dead though, try resurrecting with \"live\".\n");
		}
		return;
	}

	world = IDD->find_object_by_name("planets:aerth");

	if (!world) {
		send_out("BUG: Starting world not found, yell at a wizard.\n");
		return;
	}

	send_out("Creating body...\n");
	body = GAME_INITD->create_thing();

	body->set_object_name("players:" + name);
	body->set_id(name);
	body->set_archetype(template);

	body->set_local_mass(1.0);
	body->set_local_capacity(1.0);
	body->set_local_max_mass(1.0);

	body->add_local_detail(nil);
	body->add_local_snoun(nil, "body");
	body->add_local_pnoun(nil, "bodies");
	body->add_local_adjective(nil, name);

	body->initialize_character(10 + random(11), random(11), 30 + random(11));

	ghost->move(body);
	ghost->possess(body);
	body->move(world);

	emit_from(body, body, " ", ({ "revive", "revives" }), "!");
}

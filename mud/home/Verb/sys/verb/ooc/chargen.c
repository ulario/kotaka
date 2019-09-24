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
#include <kernel/version.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/utility.h>
#include <kotaka/paths/verb.h>
#include <kotaka/version.h>
#include <game/paths.h>
#include <status.h>

inherit "/lib/math/random";
inherit "/lib/string/case";
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Chargen";
}

string *query_help_contents()
{
	return ({
		"Chargen creates a character.",
		"For now, it just creates an instance of the human class and dumps you in the world."
	});
}

void initialize_combatant(object fighter)
{
	fighter->clear_character(); /* also wipes living */
	fighter->initialize_character(5 + random(6), random(6), 20 + random(11));
	fighter->initialize_living();
}

void main(object actor, mapping roles)
{
	string name;
	object body;
	object user;
	object human;
	object world;

	user = query_user();
	name = user->query_name();

	if (!name) {
		send_out("You must log in before creating a character.\n");
		return;
	}

	if (IDD->find_object_by_name("players:" + name)) {
		send_out("You already have a character.\n");
		return;
	}

	human = IDD->find_object_by_name("class:race:humanoid:human");

	if (!human) {
		send_out("Yell at a wizard, there's no human archetype to spawn your character from.\n");
		return;
	}

	body = GAME_INITD->create_thing();

	body->set_density(1.0);
	body->set_mass(100.0 + rnd() * 10.0);

	body->set_property("id", name);
	body->set_property("local_snouns", ({ name }) );
	body->set_property("is_proper", 1);
	body->set_property("brief", to_title(name));

	body->set_archetype(IDD->find_object_by_name("class:race:humanoid:human"));
	body->set_object_name("players:" + name);

	send_out("Created " + TEXT_SUBD->generate_brief_definite(body) + ".\n");

	initialize_combatant(body);

	world = IDD->find_object_by_name("planets:aerth");

	if (world) {
		body->move(world);
	} else {
		send_out("Warning: start room not found, your character's body was spawned in the void.\n");
		send_out("Please contact a wizard to move you to the game world.\n");
	}
}

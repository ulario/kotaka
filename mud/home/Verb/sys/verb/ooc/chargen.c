/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2020, 2021, 2022  Raymond Jennings
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

void main(object actor, mapping roles)
{
	int newchar;
	object body, ghost, human, template, user, world;
	string name;

	if (roles["raw"]) {
		send_out("Usage: chargen\n");
		return;
	}

	user = query_user();
	name = user->query_name();

	if (!name) {
		send_out("You must log in before creating a character.\n");
		return;
	}

	world = IDD->find_object_by_name("planets:aerth");

	if (!world) {
		send_out("BUG: Starting world not found, yell at a wizard.\n");
		return;
	}

	human = IDD->find_object_by_name("class:race:human");

	if (!human) {
		send_out("BUG: human race not found, yell at a wizard.\n");
		return;
	}

	template = IDD->find_object_by_name("templates:" + name);

	if (!template) {
		float factor;

		factor = 1.0 + (rnd() - rnd()) * 0.1;

		send_out("Creating template...\n");
		template = GAME_INITD->create_thing();

		template->set_object_name("templates:" + name);
		template->set_id(name);

		template->set_archetype(human);
		template->set_mass(100.0 * factor);
		template->set_capacity(1.0 * factor);
		template->set_max_mass(100.0 * factor);

		template->add_local_detail(nil);
		template->add_local_snoun(nil, name);
		template->set_local_description(nil, "brief", to_title(name));
		template->set_local_property("is_proper", 1);

		newchar = 1;
	}

	ghost = IDD->find_object_by_name("ghosts:" + name);

	if (!ghost) {
		if (newchar) {
			send_out("Creating ghost...\n");
			ghost = GAME_INITD->create_thing();
			ghost->set_object_name("ghosts:" + name);
			ghost->set_id(name);
			ghost->set_archetype(template);
			ghost->set_virtual(1);

			ghost->add_local_detail(nil);
			ghost->add_local_snoun(nil, "ghost");
			ghost->add_local_pnoun(nil, "ghost");
			ghost->set_local_adjectives(nil, ({ "ghost", name }) );
			ghost->set_local_description(nil, "brief", to_title(name) + "'s ghost");
		} else {
			send_out("Tragedy has struck, your soul has been annihilated.\n");
			return;
		}
	}

	body = IDD->find_object_by_name("players:" + name);

	if (!body) {
		if (newchar) {
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
		} else {
			send_out("Alas, your body is gone.\n");
			return;
		}
	}

	if (ghost->query_possessee() != body) {
		send_out("Yikes, you're having an out of body experience right now!\n");
		send_out("If you're not supposed to be severed from your body, yell at a wizard.\n");
	}
}

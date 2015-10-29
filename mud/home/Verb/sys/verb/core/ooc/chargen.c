/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2015  Raymond Jennings
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
#include <kotaka/paths/string.h>
#include <kotaka/paths/utility.h>
#include <kotaka/paths/verb.h>
#include <kotaka/version.h>
#include <game/paths.h>
#include <status.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	string name;
	object body;
	object user;

	user = query_user();
	name = user->query_name();

	if (!name) {
		send_out("You must log in before creating a character.\n");
		return;
	}

	if (CATALOGD->lookup_object("players:" + name)) {
		send_out("You already have a character.\n");
		return;
	}

	body = GAME_INITD->create_thing();

	body->set_density(1.0);
	body->set_mass(100.0 + MATHD->rnd() * 10.0);

	body->set_property("id", name);
	body->set_property("local_snouns", ({ name }) );
	body->set_property("is_proper", 1);
	body->set_property("brief", STRINGD->to_title(name));

	body->add_archetype(CATALOGD->lookup_object("class:race:humanoid:human"));
	body->set_object_name("players:" + name);
	send_out("Created " + TEXT_SUBD->generate_brief_definite(body) + ".\n");
}

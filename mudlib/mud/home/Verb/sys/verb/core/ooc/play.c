/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/paths/kotaka.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
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

		body = CATALOGD->lookup_object(roles["raw"]);
	}

	if (!body) {
		body = CATALOGD->lookup_object("players:" + name);
	}

	if (!body) {
		world = GAME_INITD->query_world();

		body = GAME_INITD->create_object();

		body->set_density(1.0);
		body->set_mass(100.0 + MATHD->rnd() * 10.0);

		body->set_property("id", name);
		body->set_property("local_snouns", ({ name }) );
		body->set_property("is_definite", 1);
		body->set_property("brief", STRINGD->to_title(name));
		body->add_archetype(CATALOGD->lookup_object("class:race:humanoid:human"));
		body->set_object_name("players:" + name);
		body->move(world);
		send_out("Created " + TEXT_SUBD->generate_brief_definite(body) + ".\n");
	}

	oldbody = user->query_body();

	if (oldbody) {
		send_out("Unselected " + TEXT_SUBD->generate_brief_definite(oldbody) + ".\n");

		mobiles = oldbody->query_property("mobiles");
		mobiles -= ({ user, nil });

		if (!sizeof(mobiles)) {
			mobiles = nil;
		}

		oldbody->set_property("mobiles", mobiles);
	}

	mobiles = body->query_property("mobiles");
	mobiles |= ({ user });
	body->set_property("mobiles", mobiles);

	user->set_body(body);
	send_out("Selected " + TEXT_SUBD->generate_brief_definite(body) + ".\n");
}

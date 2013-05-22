/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012-2013  Raymond Jennings
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
#include <text/paths.h>
#include <game/paths.h>

inherit LIB_VERB;

/* ({ role, prepositions, raw }) */
mixed **query_roles()
{
	return ({ });
}

void main(object actor, mapping roles)
{
	string name;
	object world;
	object body;
	object *inv;

	name = query_user()->query_name();

	if (!(name = query_user()->query_name())) {
		send_out("You aren't logged in.\n");
		return;
	}

	body = CATALOGD->lookup_object("players:" + name);

	if (!body) {
		world = GAME_INITD->query_world();

		body = GAME_INITD->create_object();

		body->set_density(1.0);
		body->set_mass(100.0 + SUBD->rnd() * 10.0);

		body->set_property("id", name);
		body->set_property("local_nouns", ({ name }) );
		body->add_archetype(CATALOGD->lookup_object("animal:human"));
		body->set_object_name("players:" + name);
		body->move(world);
	}

	query_user()->set_body(body);
}

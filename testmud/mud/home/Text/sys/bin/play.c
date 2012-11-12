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
#include <kotaka/paths.h>
#include <text/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	object world;
	object body;

	object *inv;

	if (!query_user()->query_name()) {
		send_out("You aren't logged in.\n");
		return;
	}

	world = GAME_INITD->query_world();
	inv = world->query_inventory();

	if (sizeof(inv)) {
		body = inv[random(sizeof(inv))];
	} else {
		body = GAME_INITD->create_object();

		body->set_property("id", "human");
	}


	query_ustate()->set_body(body);
}

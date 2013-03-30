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
#include <catalog/paths.h>

inherit LIB_VERB;

void main(object actor, string args)
{
	object shack, env;

	if (!actor) {
		send_out("Odd...without a body you have no hands.\nPlease inhabit an object first.\n");
		return;
	}

	env = actor->query_environment();

	shack = GAME_INITD->create_object();
	shack->set_property("id", "shack");
	shack->add_archetype(CATALOGD->lookup_object("shack"));
	shack->move(env);

	shack->set_x_position(actor->query_x_position());
	shack->set_y_position(actor->query_y_position());
	shack->set_z_position(actor->query_z_position());

	shack->query_property("event:create")->on_create(shack);

	send_out("You build a shack.\n");
}

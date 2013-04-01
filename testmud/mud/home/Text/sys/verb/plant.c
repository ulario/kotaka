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
#include <catalog/paths.h>

inherit LIB_VERB;

void main(object actor, string args)
{
	object tree, env;

	if (!actor) {
		send_out("Odd...without a body you have no hands.\nPlease inhabit an object first.\n");
		return;
	}

	env = actor->query_environment();

	tree = GAME_INITD->create_object();
	tree->set_property("id", "tree");
	tree->add_archetype(CATALOGD->lookup_object("tree"));
	tree->move(env);

	tree->set_x_position(actor->query_x_position());
	tree->set_y_position(actor->query_y_position());
	tree->set_z_position(actor->query_z_position());
	tree->query_property("event:create")->on_create(tree);

	send_out("You plant a tree.\n");
}

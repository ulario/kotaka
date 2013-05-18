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

void main(object actor, mixed *tree)
{
	object gtree, env;
	string args;

	args = fetch_raw(tree);

	env = actor->query_environment();

	gtree = GAME_INITD->create_object();
	gtree->set_property("id", "tree");
	gtree->add_archetype(CATALOGD->lookup_object("scenery:tree"));
	gtree->move(env);

	gtree->set_x_position(actor->query_x_position());
	gtree->set_y_position(actor->query_y_position());
	gtree->set_z_position(actor->query_z_position());
	gtree->query_property("event:create")->on_create(gtree);

	send_out("You plant a tree.\n");
}

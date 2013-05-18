/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <text/paths.h>

inherit LIB_VERB;

int query_raw()
{
	return 0;
}

void main(object actor, mixed *tree)
{
	mixed *vp;
	mixed *np;
	string evoke;
	object *candidates;

	vp = tree[1];

	ASSERT(vp[0] == "V");
	ASSERT(vp[1] == "drop");
	np = vp[2];
	ASSERT(sizeof(vp) == 3);
	evoke = fetch_evoke(tree);

	if (np[0] == "P") {
		send_out("Your grammar stinks.\n");
		return;
	}

	if (!actor->query_environment()) {
		send_out("In the formless void of space?  You're kidding, right?\n");
		return;
	}

	candidates = bind_objects(actor->query_environment()->query_inventory(), np);

	if (!sizeof(candidates)) {
		send_out("No such thing is here for you to take.\n");
		return;
	}

	send_out("You take " + candidates[0]->query_property("brief") + ".\n");
	candidates[0]->move(actor);
}

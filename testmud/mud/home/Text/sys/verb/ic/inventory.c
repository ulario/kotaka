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
#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <text/paths.h>

inherit LIB_VERB;

int query_raw()
{
	return 0;
}

void main(object actor, mapping roles)
{
	object *inv;
	int sz;

	inv = actor->query_inventory();
	sz = sizeof(inv);

	if (sz) {
		int i;

		send_out("Your inventory:\n\n");

		for (i = 0; i < sz; i++) {
			send_out(generate_brief_indefinite(inv[i]) + "\n");
		}
	} else {
		send_out("You have nothing.\n");
	}
}

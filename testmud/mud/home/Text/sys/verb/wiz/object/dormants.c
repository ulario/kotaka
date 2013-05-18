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

inherit LIB_VERB;

void main(object actor, mixed *tree)
{
	object arr;
	int sz, i;
	string args;

	args = fetch_raw(tree);

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to list dormants.\n");
		return;
	}

	arr = OBJECTD->query_dormant();

	if (!(sz = arr->get_size())) {
		send_out("There are no dormant LPC source files.\n");
		return;
	}

	for (i = 0; i < sz; i++) {
		send_out(arr->get_element(i) + "\n");
	}
}

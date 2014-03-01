/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2011, 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	object arr;
	int sz, i;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to list dormants.\n");
		return;
	}

	arr = OBJECTD->query_dormant();

	if (!(sz = arr->query_size())) {
		send_out("There are no dormant LPC source files.\n");
		return;
	}

	send_out("These LPC files are not compiled:\n");

	for (i = 0; i < sz; i++) {
		send_out(arr->query_element(i) + "\n");
	}
}

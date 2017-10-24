/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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
	string owner;
	object first;
	object obj;

	if (query_user()->query_class() < 2) {
		send_out("Only a wizard can list objregd.\n");
		return;
	}

	owner = roles["raw"];

	first = KERNELD->first_link(owner == "Ecru" ? nil : owner);

	if (!first) {
		send_out(owner + " owns no objects.\n");
		return;
	}

	obj = first;

	send_out("Objects owned by " + owner + ":\n");

	do {
		send_out(object_name(obj) + "\n");

		obj = KERNELD->next_link(obj);
	} while (obj != first);
}

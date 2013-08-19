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
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>

inherit LIB_RAWVERB;

void main(object actor, string args)
{
	string owner;
	mapping seen;
	object first;
	object obj;

	if (query_user()->query_class() < 2) {
		send_out("Only a wizard can list objregd.\n");
		return;
	}

	seen = ([ ]);
	first = KERNELD->first_link(args);

	if (!first) {
		send_out(args + " owns no objects.\n");
		return;
	}

	obj = first;

	send_out("Objects owned by " + args + ":\n");

	do {
		send_out(object_name(obj) + "\n");

		seen[obj] = 1;
		obj = KERNELD->next_link(obj);

		if (!obj) {
			send_out("nil\n");
			break;
		}
	} while (!seen[obj]);
}

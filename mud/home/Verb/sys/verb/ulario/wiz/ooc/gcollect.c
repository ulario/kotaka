/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/verb.h>
#include <game/paths.h>
#include <status.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	object cinfo;
	object first;
	object obj;
	object list;
	int sz, i;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to do a garbage check.\n");
		return;
	}

	cinfo = CLONED->query_clone_info(status("/home/Game/obj/thing", O_INDEX));

	first = cinfo->query_first_clone();
	obj = first;
	list = new_object(BIGSTRUCT_ARRAY_LWO);
	list->claim();

	do {
		if (!obj->query_environment() && !obj->query_object_name()) {
			list->push_back(obj);
		}

		obj = obj->query_next_clone();
	} while (obj != first);

	sz = list->query_size();

	for (i = 0; i < sz; i++) {
		GAME_INITD->destroy_object(list->query_element(i));
	}

	send_out("Destroyed " + sz + " orphans.\n");
}
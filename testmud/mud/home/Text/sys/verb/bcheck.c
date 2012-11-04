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
#include <kotaka/log.h>
#include <game/paths.h>
#include <text/paths.h>
#include <status.h>
#include <kotaka/bigstruct.h>

inherit LIB_VERB;

private void list_append(object list, object obj)
{
	object *inv;
	int sz, i;

	list->push_back(obj);

	inv = obj->query_inventory();
	sz = sizeof(inv);

	for (i = 0; i < sz; i++) {
		list_append(list, inv[i]);
	}
}

void main(object actor, string args)
{
	object world;
	object list;

	int ticks, ticks2;
	int i, sz;

	list = new_object(BIGSTRUCT_ARRAY_LWO);

	list_append(list, world = GAME_INITD->query_world());

	sz = list->get_size();

	BULKD->reset();

	for (i = 0; i < sz; i++) {
		object obj;

		obj = list->get_element(i);

		if (obj->query_bulk_queued()) {
			LOGD->post_message("bulk", LOG_DEBUG, object_name(obj) + " is still queued!");
		}
	}
}

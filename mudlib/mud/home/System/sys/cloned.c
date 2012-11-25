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
#include <kotaka/bigstruct.h>
#include <kotaka/log.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <status.h>
#include <type.h>

inherit SECOND_AUTO;

object db;
int busy;
object *overflow;

private void discover_clones();

static void create()
{
	overflow = ({ });

	db = clone_object(BIGSTRUCT_MAP_OBJ);
	db->set_type(T_INT);

	discover_clones();
}

void add_clone(object obj)
{
	object cinfo;
	int index;

	ACCESS_CHECK(previous_program() == OBJECTD || previous_program() == CLONED);

	if (!db) {
		return;
	}

	if (busy) {
		overflow += ({ obj });
		return;
	}

	index = status(obj, O_INDEX);
	cinfo = db->get_element(index);

	LOGD->post_message("clone", LOG_DEBUG, "Adding clone: " + object_name(obj));

	if (!cinfo) {
		busy = 1;
		db->set_element(index, cinfo = new_object(CLONE_INFO));
		busy = 0;
		cinfo->set_first_clone(obj);
		obj->set_prev_clone(obj);
		obj->set_next_clone(obj);
	} else {
		object first;
		object next;

		first = cinfo->query_first_clone();
		next = first->query_next_clone();

		first->set_next_clone(obj);
		next->set_prev_clone(obj);

		obj->set_prev_clone(first);
		obj->set_next_clone(next);
	}

	cinfo->add_clone(obj);

	while (sizeof(overflow)) {
		object over;

		over = overflow[0];
		overflow = overflow[1 ..];
		add_clone(over);
	}
}

void remove_clone(object obj)
{
	object cinfo;
	int index;
	object prev;
	object next;

	ACCESS_CHECK(previous_program() == OBJECTD);

	index = status(obj, O_INDEX);
	cinfo = db->get_element(index);
	cinfo->remove_clone(obj);

	if (obj == obj->query_next_clone()) {
		/* last clone */
		db->set_element(index, nil);
		return;
	}

	if (obj == cinfo->query_first_clone()) {
		cinfo->set_first_clone(obj->query_next_clone());
	}

	prev = obj->query_prev_clone();
	next = obj->query_next_clone();

	prev->set_next_clone(next);
	next->set_prev_clone(prev);
}

private void discover_clones()
{
	string *owners;
	int i, sz;
	int count;

	owners = KERNELD->query_owners();
	sz = sizeof(owners);

	for (i = 0; i < sz; i++) {
		object first;

		first = KERNELD->first_link(owners[i]);

		if (first) {
			object obj;
			string name;

			obj = first;

			do {
				name = object_name(obj);

				if (sscanf(name, "%*s#%*d")) {
					add_clone(obj);
				}
				obj = KERNELD->next_link(obj);
			} while (obj != first);
		}
	}
}

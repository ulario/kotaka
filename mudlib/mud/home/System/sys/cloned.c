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
#include <kotaka/assert.h>
#include <kotaka/bigstruct.h>
#include <kotaka/log.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <status.h>
#include <type.h>

inherit SECOND_AUTO;

mapping bmap;	/* keeping track of bigstruct clones */
object db;

void discover_clones();

static void create()
{
	bmap = ([ ]);

	db = clone_object(BIGSTRUCT_MAP_OBJ);
	db->set_type(T_INT);

	discover_clones();

	call_out("defragment", 5);
}

void add_clone(object obj);

static void defragment()
{
	call_out("defragment", 60);

	db->reindex();
}

void add_clone(object obj)
{
	object cinfo;
	int index;
	int is_bigstruct;

	ACCESS_CHECK(previous_program() == OBJECTD || previous_program() == CLONED);

	index = status(obj, O_INDEX);

	is_bigstruct = DRIVER->creator(object_name(obj)) == "Bigstruct";

	if (is_bigstruct) {
		cinfo = bmap[index];
	} else {
		cinfo = db->get_element(index);
	}

	if (!cinfo) {
		cinfo = new_object(CLONE_INFO);

		if (is_bigstruct) {
			bmap[index] = cinfo;
		} else {
			db->set_element(index, cinfo);
		}

		if (!sscanf(object_name(obj), "/kernel/%*s")) {
			cinfo->set_first_clone(obj);
			obj->set_prev_clone(obj);
			obj->set_next_clone(obj);
		}
	} else {
		if (!sscanf(object_name(obj), "/kernel/%*s")) {
			object first;
			object next;

			first = cinfo->query_first_clone();
			next = first->query_next_clone();

			first->set_next_clone(obj);
			next->set_prev_clone(obj);

			obj->set_prev_clone(first);
			obj->set_next_clone(next);
		}
	}

	cinfo->add_clone(obj);
}

void remove_clone(object obj)
{
	object cinfo;
	object prev;
	object next;
	int index;
	int is_bigstruct;

	ACCESS_CHECK(previous_program() == OBJECTD);

	is_bigstruct = DRIVER->creator(object_name(obj)) == "Bigstruct";

	index = status(obj, O_INDEX);

	if (is_bigstruct) {
		cinfo = bmap[index];
	} else {
		cinfo = db->get_element(index);
	}

	cinfo->remove_clone(obj);

	if (obj == obj->query_next_clone()) {
		/* last clone */
		if (is_bigstruct) {
			bmap[index] = nil;
		} else {
			db->set_element(index, nil);
		}
		return;
	}

	if (sscanf(object_name(obj), "/kernel/%*s")) {
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

void discover_clones()
{
	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	rlimits (0; -1) {
		string *owners;
		int i, sz;
		int count;
		object queue;

		owners = KERNELD->query_owners();
		sz = sizeof(owners);
		queue = new_object(BIGSTRUCT_DEQUE_LWO);

		db->clear();
		bmap = ([ ]);

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
						queue->push_back(obj);
					}

					obj = KERNELD->next_link(obj);
				} while (obj != first);
			}
		}

		while (!queue->empty()) {
			object obj;

			obj = queue->get_front();
			queue->pop_front();
			add_clone(obj);
		}
	}
}

object query_clone_info(int index)
{
	if (bmap[index]) {
		return bmap[index];
	}

	return db->get_element(index);
}

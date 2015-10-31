/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2007, 2010, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kernel/kernel.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <status.h>
#include <type.h>

#define CLONED_RESET_BUDGET 200000

inherit SECOND_AUTO;

mapping bmap;	/* keeping track of bigstruct clones */
object db;	/* keeping track of other clones */

void discover_clones();

static void create()
{
	bmap = ([ ]);
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
		cinfo = db->query_element(index);
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
		cinfo = db->query_element(index);
	}

	if (cinfo) {
		if (obj == cinfo->query_first_clone()) {
			cinfo->set_first_clone(obj->query_next_clone());
		}

		cinfo->remove_clone(obj);
	}

	if (sscanf(object_name(obj), "/kernel/%*s")) {
		return;
	}

	if (obj == obj->query_next_clone()) {
		/* last clone */
		if (is_bigstruct) {
			bmap[index] = nil;
		} else {
			db->set_element(index, nil);
		}
		return;
	}

	prev = obj->query_prev_clone();
	next = obj->query_next_clone();

	if (prev) {
		prev->set_next_clone(next);
	}

	if (next) {
		next->set_prev_clone(prev);
	}
}

void remove_program(int index)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	bmap[index] = nil;

	if (db) {
		db->set_element(index, nil);
	}
}

int count;

void discover_clones_work_gather(string *owners, object queue)
{
	ACCESS_CHECK(previous_program() == SUSPENDD);

	if (sizeof(owners)) {
		string owner;
		object first, obj;

		owner = owners[0];
		owners = owners[1 ..];

		first = KERNELD->first_link(owner);

		obj = first;

		if (first) {
			SUSPENDD->queue_work("discover_clones_work_gather_owner", owners, queue, first, obj);
		} else {
			SUSPENDD->queue_work("discover_clones_work_gather", owners, queue);
		}
	} else {
		SUSPENDD->queue_work("discover_clones_work_link", queue);
	}
}

void discover_clones_work_gather_owner(string *owners, object queue, object first, object obj)
{
	int ticks;
	int done;
	string name;

	ACCESS_CHECK(previous_program() == SUSPENDD);

	ticks = status(ST_TICKS);

	while (ticks - status(ST_TICKS) < CLONED_RESET_BUDGET) {
		name = object_name(obj);

		if (sscanf(name, "%*s#%*d")) {
			count++;
			queue->push_back(obj);
		}

		obj = KERNELD->next_link(obj);

		if (obj == first) {
			done = 1;
			break;
		}
	}

	LOGD->post_message("system", LOG_INFO, "CloneD reset: " + count + " clones discovered so far.");

	if (done) {
		SUSPENDD->queue_work("discover_clones_work_gather", owners, queue);
	} else {
		SUSPENDD->queue_work("discover_clones_work_gather_owner", owners, queue, first, obj);
	}
}

void discover_clones_work_link(object queue)
{
	int done;
	int ticks;

	ACCESS_CHECK(previous_program() == SUSPENDD);

	ticks = status(ST_TICKS);

	while (ticks - status(ST_TICKS) < CLONED_RESET_BUDGET) {
		object obj;

		if (queue->empty()) {
			done = 1;
			break;
		}

		obj = queue->query_front();
		queue->pop_front();
		add_clone(obj);
		count--;
	}

	if (!done) {
		LOGD->post_message("system", LOG_INFO, "CloneD reset: " + count + " clones left to link.");

		SUSPENDD->queue_work("discover_clones_work_link", queue);
	}
}

atomic void discover_clones()
{
	string *owners;
	int i, sz;
	int count;
	object queue;

	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	owners = KERNELD->query_owners();
	sz = sizeof(owners);
	queue = new_object(BIGSTRUCT_DEQUE_LWO);
	queue->claim();

	if (db) {
		destruct_object(db);
	}

	bmap = ([ ]);

	db = clone_object(BIGSTRUCT_MAP_OBJ);
	db->claim();
	db->set_type(T_INT);

	count = 0;

	SUSPENDD->suspend_system();
	SUSPENDD->queue_work("discover_clones_work_gather", owners, queue);
}

object query_clone_info(int index)
{
	if (bmap[index]) {
		return bmap[index];
	}

	return db->query_element(index);
}

static void destruct()
{
	if (db) {
		destruct_object(db);
	}
}

/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
#include <kernel/access.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/privilege.h>
#include <status.h>

inherit SECOND_AUTO;
inherit "~/lib/struct/multimap";

mapping patcherdb; /* ([ level : ([ index : patchers ]) ]) */
mapping patchabledb;
mapping objdb; /* ([ level : ([ index : obj ]) ]) */

int handle;
mixed *sweep_list;
mixed *nudge_list;

static void create()
{
}

/* hooks */

atomic void reset_callout()
{
	mixed **callouts;
	int sz;

	callouts = status(this_object(), O_CALLOUTS);

	for (sz = sizeof(callouts); --sz >= 0; ) {
		remove_call_out(callouts[sz][CO_HANDLE]);
	}

	handle = call_out("process", 0);
}

void reboot()
{
	ACCESS_CHECK(SYSTEM());
}

void cleanup_patch()
{
	ACCESS_CHECK(SYSTEM());
}

private void enqueue_sweep(string path)
{
	if (!sweep_list) {
		sweep_list = ({ nil, nil });
	}

	list_push_back(sweep_list, ({ path, status(ST_OTABSIZE) }) );

	if (!handle) {
		handle = call_out("process", 0);
	}
}

private void enqueue_nudge(object obj)
{
	if (!nudge_list) {
		nudge_list = ({ nil, nil });
	}

	list_push_back(nudge_list, obj);

	if (!handle) {
		handle = call_out("process", 0);
	}
}

atomic void enqueue_patchers(object master, string *patchers)
{
	int index;
	string path;
	int touchcount;

	ACCESS_CHECK(previous_program() == OBJECTD);

	touchcount = 0;
	path = object_name(master);
	index = status(master, O_INDEX);

	if (!patcherdb) {
		patcherdb = ([ ]);
	}

	set_multimap(patcherdb, index, patchers);

	if (!objdb) {
		objdb = ([ ]);
	}

	set_multimap(objdb, index, master);

	call_touch(master);
	enqueue_nudge(master);

	if (sscanf(path, "%*s" + CLONABLE_SUBDIR + "%*s")) {
		rlimits(0; -1) {
			int sz;

			for (sz = status(ST_OTABSIZE); --sz >= 0; ) {
				object obj;

				if (obj = find_object(path + "#" + sz)) {
					set_multimap(objdb, sz, obj);
					call_touch(obj);
					touchcount++;
				}
			}
		}

		enqueue_sweep(path);
	}
}

private string *lookup_patchers(int index)
{
	if (patcherdb) {
		string *patchers;

		patchers = query_multimap(patcherdb, index);

		if (patchers) {
			return patchers;
		}
	}
}

string *query_patchers(object obj)
{
	object odbv;
	string path;
	int index;
	int mindex;
	string *patchers;

	ACCESS_CHECK(SYSTEM());

	path = object_name(obj);
	mindex = status(obj, O_INDEX);

	if (!sscanf(path, "%s#%d", path, index)) {
		index = mindex;
	}

	if (objdb) {
		if (query_multimap(objdb, index)) {
			return lookup_patchers(mindex);
		}
	}

	if (patchabledb) {
		if (query_multimap(patchabledb, index)) {
			return lookup_patchers(mindex);
		}
	}
}

void clear_patch(object obj)
{
	string path;
	int index;

	ACCESS_CHECK(SYSTEM());

	path = object_name(obj);

	if (!sscanf(path, "%s#%d", path, index)) {
		index = status(obj, O_INDEX);
	}

	if (patchabledb) {
		set_multimap(patchabledb, index, nil);
	}

	if (objdb) {
		set_multimap(objdb, index, nil);
	}
}

static void nudge_object(object obj)
{
	obj->_F_dummy();
}

private int sweep(string path, int index)
{
	int goal;

	goal = index - 1000;

	if (goal < 0) {
		goal = 0;
	}

	while (--index > goal) {
		object obj;

		if (obj = find_object(path + "#" + index)) {
			enqueue_nudge(obj);
			break;
		}
	}

	return index;
}

static void process()
{
	handle = 0;

	if (nudge_list && !list_empty(nudge_list)) {
		object obj;

		handle = call_out("process", 0);

		obj = list_front(nudge_list);
		list_pop_front(nudge_list);

		if (obj) {
			obj->_F_dummy();
		}
	} else if (sweep_list && !list_empty(sweep_list)) {
		string path;
		mixed *head;
		int index;

		handle = call_out("process", 0);

		head = list_front(sweep_list);
		({ path, index }) = head;

		index = sweep(path, index);

		if (index) {
			head[1] = index;
		} else {
			list_pop_front(sweep_list);
		}
	} else {
		sweep_list = nil;
		nudge_list = nil;
	}
}

void reset()
{
	mixed **callouts;
	int sz;

	ACCESS_CHECK(SYSTEM() || KADMIN());

	handle = 0;

	sweep_list = nil;
	nudge_list = nil;
	patcherdb = nil;
	patchabledb = nil;
	objdb = nil;

	callouts = status(this_object(), O_CALLOUTS);

	for (sz = sizeof(callouts); --sz >= 0; ) {
		remove_call_out(callouts[sz][CO_HANDLE]);
	}
}

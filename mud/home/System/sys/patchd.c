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

object pflagdb;

mixed **patch_queue; /* ({ obj }) */
mixed **sweep_queue; /* ({ path, master_index, clone_index }) */

int query_marked(object obj);

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

private void touch_one(object obj)
{
	int index;

	if (!sscanf(object_name(obj), "%*s#%d", index)) {
		index = status(obj, O_INDEX);
	}

	set_multimap(objdb, index, obj);

	call_touch(obj);
	enqueue_nudge(obj);
}

private int touch_all(string path)
{
	int touchcount;

	rlimits(0; -1) {
		int sz;

		for (sz = status(ST_OTABSIZE); --sz >= 0; ) {
			object obj;

			if (obj = find_object(path + "#" + sz)) {
				call_touch(obj);
				set_multimap(objdb, sz, obj);
				touchcount++;
			}
		}
	}

	enqueue_sweep(path);

	return touchcount;
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

	touch_one(master);

	if (sscanf(path, "%*s" + CLONABLE_SUBDIR + "%*s")) {
		object pinfo;

		pinfo = OBJECTD->query_program_info(index);

		if (pinfo) {
			object *clones;
			int sz;

			if (pinfo->query_clone_count()) {
				clones = pinfo->query_clones();

				if (clones) {
					int sz;

					for (sz = sizeof(clones); --sz >= 0; ) {
						touch_one(clones[sz]);
					}
				} else {
					LOGD->post_message("system", LOG_WARNING, "Clone overflow for " + path + ", sweeping");
					touch_all(path);
				}
			}
		} else {
			LOGD->post_message("system", LOG_WARNING, "Missing program info for " + path + ", not patching");
		}
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

	if (pflagdb) {
		pflagdb->set_element(index, nil);
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
			obj->_F_dummy();
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

		if (list_empty(nudge_list)) {
			nudge_list = nil;
		}

		if (!obj) {
			return;
		}

		obj->_F_dummy();
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

			if (list_empty(sweep_list)) {
				sweep_list = nil;
			}
		}
	} else if (patch_queue) {
		object obj;

		handle = call_out("process", 0);

		obj = list_front(patch_queue);
		list_pop_front(patch_queue);

		if (list_empty(patch_queue)) {
			patch_queue = nil;
		}

		if (!obj) {
			return;
		}

		obj->_F_dummy();
	} else if (sweep_queue) {
		mixed *head;
		string path;
		int master_index;
		int clone_index;
		int goal;

		handle = call_out("process", 0);

		head = list_front(sweep_queue);

		({ path, master_index, clone_index }) = head;

		goal = (clone_index - 1) & ~1023;

		do {
			object clone;

			clone_index--;

			clone = find_object(path + "#" + clone_index);

			if (clone && status(clone, O_INDEX) == master_index && query_marked(clone)) {
				clone->_F_dummy();
				break;
			}
		} while (clone_index > goal);

		if (clone_index) {
			head[2] = clone_index;
		} else {
			list_pop_front(sweep_queue);

			if (list_empty(sweep_queue)) {
				sweep_queue = nil;
			}
		}
	} else {
		pflagdb = nil;
		sweep_queue = nil;
		patch_queue = nil;
		sweep_list = nil;
		nudge_list = nil;
	}
}

private void queue_patch(object obj)
{
	if (!patch_queue) {
		patch_queue = ({ nil, nil });
	}

	if (!handle) {
		handle = call_out("process", 0);
	}

	list_push_back(patch_queue, obj);
}

private void queue_sweep(string path, int master_index)
{
	if (!sweep_queue) {
		sweep_queue = ({ nil, nil });
	}

	if (!handle) {
		handle = call_out("process", 0);
	}

	list_push_back(sweep_queue, ({ path, master_index, status(ST_OTABSIZE) }));
}

void mark_patch(string path)
{
	int index;
	object pinfo;
	object master;

	ACCESS_CHECK(previous_program() == OBJECTD);

	master = find_object(path);
	index = status(master, O_INDEX);
	pinfo = OBJECTD->query_program_info(index);

	if (!pflagdb) {
		pflagdb = new_object(SPARSE_ARRAY);
	}

	pflagdb->set_element(index, master);
	queue_patch(master);

	if (pinfo->query_clone_count()) {
		object *clones;

		clones = pinfo->query_clones();

		if (clones) {
			int sz;

			for (sz = sizeof(clones); --sz >= 0; ) {
				object clone;

				clone = clones[sz];

				pflagdb->set_element(sz, clone);
				queue_patch(clone);
			}
		} else {
			int sz;

			LOGD->post_message("system", LOG_WARNING, "Clone overflow for " + path + ", sweeping");

			for (sz = status(ST_OTABSIZE); --sz >= 0; ) {
				object clone;

				clone = find_object(path + "#" + sz);

				if (clone && status(clone, O_INDEX) == index) {
					pflagdb->set_element(sz, clone);
				}
			}

			queue_sweep(path, index);
		}
	}
}

int query_marked(object obj)
{
	int index;

	if (!sscanf(object_name(obj), "%*s#%d", index)) {
		index = status(obj, O_INDEX);
	}

	if (pflagdb && pflagdb->query_element(index)) {
		return 1;
	}

	if (objdb && query_multimap(objdb, index)) {
		return 1;
	}

	return 0;
}

void clear_mark(object obj)
{
	ACCESS_CHECK(SYSTEM());

	clear_patch(obj);
}

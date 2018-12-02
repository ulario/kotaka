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
inherit "~/lib/struct/list";

object pflagdb;
mixed **patch_queue; /* ({ obj }) */
mixed **sweep_queue; /* ({ path, master_index, clone_index }) */

int query_marked(object obj);

private void queue_patch(object obj)
{
	if (!patch_queue) {
		patch_queue = ({ nil, nil });
	}

	list_push_back(patch_queue, obj);
}

private void queue_sweep(string path, int master_index)
{
	if (!sweep_queue) {
		sweep_queue = ({ nil, nil });
	}

	list_push_back(sweep_queue, ({ path, master_index, status(ST_OTABSIZE) }));
}

static void create()
{
}

static void process()
{
	if (patch_queue) {
		object obj;

		call_out("process", 0);

		obj = list_front(patch_queue);
		list_pop_front(patch_queue);

		if (list_empty(patch_queue)) {
			patch_queue = nil;
		}

		if (obj) {
			obj->_F_dummy();
		}
	} else if (sweep_queue) {
		mixed *head;
		string path;
		int master_index;
		int clone_index;
		int ticks;

		call_out("process", 0);

		head = list_front(sweep_queue);

		({ path, master_index, clone_index }) = head;

		ticks = status(ST_TICKS);

		do {
			object clone;

			clone_index--;

			clone = find_object(path + "#" + clone_index);

			if (clone && status(clone, O_INDEX) == master_index && query_marked(clone)) {
				clone->_F_dummy();

				break;
			}
		} while (clone_index > 0 && status(ST_TICKS) + 100000 > ticks);

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
	}
}

void mark_patch(string path, varargs int clear)
{
	int index;
	object pinfo;
	object master;

	ACCESS_CHECK(SYSTEM());

	if (!pflagdb) {
		pflagdb = new_object(SPARSE_ARRAY);
	}

	master = find_object(path);
	index = status(master, O_INDEX);
	pinfo = OBJECTD->query_program_info(index);

	pflagdb->set_element(index, master);
	queue_patch(master);

	if (pinfo->query_clone_count()) {
		object *clones;

		clones = pinfo->query_clones();

		if (clones) {
			int sz;

			for (sz = sizeof(clones); --sz >= 0; ) {
				object clone;
				int cindex;

				clone = clones[sz];
				sscanf(object_name(clone), "%*s#%d", cindex);

				if (clear) {
					pflagdb->set_element(cindex, nil);
				} else {
					pflagdb->set_element(cindex, clone);
					call_touch(clone);
					queue_patch(clone);
				}
			}
		} else {
			int sz;

			LOGD->post_message("system", LOG_WARNING, "Clone overflow for " + path + ", sweeping");

			for (sz = status(ST_OTABSIZE); --sz >= 0; ) {
				object clone;

				clone = find_object(path + "#" + sz);

				if (clone && status(clone, O_INDEX) == index) {
					if (clear) {
						pflagdb->set_element(sz, nil);
					} else {
						call_touch(clone);
						pflagdb->set_element(sz, clone);
					}
				}
			}

			if (!clear) {
				queue_sweep(path, index);
			}
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

	return 0;
}

void clear_mark(object obj)
{
	string path;
	int index;

	ACCESS_CHECK(SYSTEM());

	path = object_name(obj);

	if (!sscanf(path, "%s#%d", path, index)) {
		index = status(obj, O_INDEX);
	}

	if (pflagdb) {
		pflagdb->set_element(index, nil);
	}
}

void reset()
{
	ACCESS_CHECK(SYSTEM() || CODE());

	wipe_callouts();

	pflagdb = nil;
	patch_queue = nil;
	sweep_queue = nil;
}

/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2019  Raymond Jennings
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
#include <status.h>
#include <type.h>
#include <kernel/access.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;
inherit "~/lib/struct/list";
inherit "~/lib/struct/sparsearray";

int query_marked(object obj);

mixed pflagdb;
mixed **patch_queue; /* ({ obj }) */
mixed **sweep_queue; /* ({ path, master_index, clone_index }) */

private void convert_pflagdb()
{
	switch(typeof(pflagdb)) {
	case T_NIL:
		pflagdb = ([ ]);

	case T_MAPPING:
		break;

	case T_OBJECT:
		rlimits (0; -1) {
			mapping newdb;
			mixed **indices;

			indices = pflagdb->query_indices();
			newdb = ([ ]);

			while (!list_empty(indices)) {
				int index;
				object obj;

				index = list_front(indices);
				list_pop_front(indices);

				obj = pflagdb->query_element(index);
				sparsearray_set_element(newdb, index, obj);
			}

			pflagdb = newdb;
		}
	}
}

private void enqueue_patch(object obj)
{
	if (!patch_queue) {
		patch_queue = ({ nil, nil });
	}

	list_push_back(patch_queue, obj);

	call_out_unique("process", 0);
}

private void enqueue_sweep(string path, int mindex)
{
	if (!sweep_queue) {
		sweep_queue = ({ nil, nil });
	}

	list_push_back(sweep_queue, ({ path, mindex, 0, status(ST_OTABSIZE) }));

	call_out_unique("process", 0);
}

static void process()
{
	if (patch_queue) {
		object obj;

		call_out_unique("process", 0);

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
		int mindex;
		int cindex;
		int otabsize;
		int ticks;
		object clone;

		call_out_unique("process", 0);

		head = list_front(sweep_queue);
		ticks = status(ST_TICKS) - 200000;

		switch(sizeof(head)) {
		case 3: /* ({ path, mindex, cindex }) */
			({ path, mindex, cindex }) = head;

			while (cindex && status(ST_TICKS) > ticks) {
				cindex--;

				clone = find_object(path + "#" + cindex);

				if (clone && status(clone, O_INDEX) == mindex && query_marked(clone)) {
					clone->_F_dummy();
				}
			}

			if (cindex) {
				head[2] = cindex;
			} else {
				LOGD->post_message("system", LOG_NOTICE, "Completed backward sweep of " + path);
				list_pop_front(sweep_queue);
			}
			break;

		case 4: /* ({ path, mindex, cindex, otabsize }) */
			({ path, mindex, cindex, otabsize }) = head;

			while (cindex < otabsize && status(ST_TICKS) > ticks) {
				clone = find_object(path + "#" + cindex++);

				if (clone && status(clone, O_INDEX) == mindex && query_marked(clone)) {
					clone->_F_dummy();
				}
			}

			if (cindex < otabsize) {
				head[2] = cindex;
				LOGD->post_message("debug", LOG_DEBUG, "Clone sweep in progress of " + path + " at " + cindex + " of " + otabsize);
			} else {
				LOGD->post_message("system", LOG_NOTICE, "Completed clone sweep of " + path);
				list_pop_front(sweep_queue);
			}
		}

		if (list_empty(sweep_queue)) {
			sweep_queue = nil;
		}
	}
}

void mark_patch(string path, varargs int clear)
{
	int index;
	object pinfo;
	object master;

	ACCESS_CHECK(SYSTEM());

	master = find_object(path);
	index = status(master, O_INDEX);
	pinfo = OBJECTD->query_program_info(index);

	if (!pflagdb) {
		pflagdb = ([ ]);
	}

	sparsearray_set_element(pflagdb, index, master);
	call_touch(master);
	enqueue_patch(master);

	call_out_unique("process", 0);

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
					sparsearray_set_element(pflagdb, cindex, nil);
				} else {
					sparsearray_set_element(pflagdb, cindex, clone);
					call_touch(clone);
					enqueue_patch(clone);
				}
			}
		} else {
			int sz;
			int marks;

			for (sz = status(ST_OTABSIZE); --sz >= 0; ) {
				object clone;

				clone = find_object(path + "#" + sz);

				if (!clone) {
					continue;
				}

				if (status(clone, O_INDEX) != index) {
					continue;
				}

				if (clear) {
					sparsearray_set_element(pflagdb, sz, nil);
				} else {
					marks++;
					call_touch(clone);
					sparsearray_set_element(pflagdb, sz, clone);
				}
			}

			if (!clear) {
				enqueue_sweep(path, index);
			}

			LOGD->post_message("system", LOG_NOTICE, "Marked " + marks + " clones of " + path + " for upgrade");
		}
	}
}

int query_marked(object obj)
{
	int index;

	if (!sscanf(object_name(obj), "%*s#%d", index)) {
		index = status(obj, O_INDEX);
	}

	if (pflagdb == nil) {
		return 0;
	}

	if (typeof(pflagdb) != T_MAPPING) {
		convert_pflagdb();
	}

	return !!sparsearray_query_element(pflagdb, index);
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

	if (pflagdb == nil) {
		return;
	}

	if (typeof(pflagdb) != T_MAPPING) {
		convert_pflagdb();
	}

	sparsearray_set_element(pflagdb, index, nil);
}

void reset()
{
	ACCESS_CHECK(SYSTEM() || CODE());

	wipe_callouts();

	pflagdb = nil;
	patch_queue = nil;
	sweep_queue = nil;
}

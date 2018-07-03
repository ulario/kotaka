/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017, 2018  Raymond Jennings
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
inherit "~/lib/system/struct/maparr";

mapping objdb;		/* ([ index : obj ]) */
mapping patchdb;	/* ([ index : patchers ]) */

mapping patcherdb;	/* ([ level : ([ index : patchers ]) ]) */
mapping patchabledb;	/* ([ level : ([ index : obj ]) ]) */

static void create()
{
}

/* hooks */

void upgrade()
{
	ACCESS_CHECK(SYSTEM());
}

void reboot()
{
	ACCESS_CHECK(SYSTEM());
}

void cleanup_patch()
{
	ACCESS_CHECK(SYSTEM());
}

private void set_multimap(mapping multimap, int index, mixed value)
{
	int shift;
	int level;

	mapping submap;

	shift = index;

	while (shift & ~255) {
		shift >>= 8;
		level++;
	}

	submap = multimap[level];

	if (!submap) {
		multimap[level] = submap = ([ ]);
	}

	while (level) {
		shift = index & (-1 << (level << 3));

		if (!submap[shift]) {
			submap[shift] = ([ ]);
		}

		submap = submap[shift];
		level--;
	}

	submap[index] = value;
}

private mixed query_multimap(mapping multimap, int index)
{
	int shift;
	int level;

	mapping submap;

	shift = index;

	while (shift & ~255) {
		shift >>= 8;
		level++;
	}

	submap = multimap[level];

	if (!submap) {
		return nil;
	}

	while (level) {
		shift = index & (-1 << (level << 3));

		if (!submap[shift]) {
			return nil;
		}

		submap = submap[shift];
		level--;
	}

	return submap[index];
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
	if (!patchabledb) {
		patchabledb = ([ ]);
	}

	set_multimap(patcherdb, index, patchers);
	set_multimap(patchabledb, index, master);

	call_touch(master);

	if (sscanf(path, "%*s" + CLONABLE_SUBDIR + "%*s")) {
		rlimits(0; -1) {
			int sz;
			int time;

			sz = status(ST_OTABSIZE);

			LOGD->post_message("system", LOG_NOTICE, "Checking " + path);

			time = time();

			while (sz >= 0) {
				int bsz;
				int time2;

				bsz = sz;
				bsz -= 1;
				bsz &= ~65535;

				for (; --sz >= bsz; ) {
					object obj;

					if (obj = find_object(path + "#" + sz)) {
						set_multimap(patchabledb, sz, obj);
						call_touch(obj);
						touchcount++;
					}

				}

				time2 = time();

				if (time != time2) {
					time = time2;

					LOGD->post_message("system", LOG_NOTICE, "Checked #" + sz);
				}
			}
		}

		call_out("sweep", 0, path);
	} else {
		call_out("nudge_object", 0, master);
	}

	if (touchcount) {
		LOGD->post_message("system", LOG_NOTICE, "Queued " + path + " and " + touchcount + " clones for patching");
	} else {
		LOGD->post_message("system", LOG_NOTICE, "Queued " + path + " for patching");
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

	if (!query_multimap(patchabledb, index)) {
		if (!query_multilevel_map_arr(objdb, 3, index)) {
			return nil;
		}
	}

	patchers = query_multimap(patcherdb, mindex);

	if (patchers) {
		return patchers;
	}

	patchers = query_multilevel_map_arr(patchdb, 3, mindex);

	if (patchers) {
		return patchers;
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

	objdb = set_multilevel_map_arr(objdb, 3, index, nil);
	set_multimap(patchabledb, index, nil);
}

static void nudge_object(object obj)
{
	obj->_F_dummy();
}

static void sweep(string path, varargs int index)
{
	if (index < status(ST_OTABSIZE)) {
		object obj;

		call_out("sweep", 0, path, index + 1);

		if (obj = find_object(path + "#" + index)) {
			obj->_F_dummy();
		}
	} else {
		LOGD->post_message("system", LOG_NOTICE, "Completed touch sweep for " + path);
	}
}

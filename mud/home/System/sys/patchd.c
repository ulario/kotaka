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

object patch;		/* old patchdb (deprecated) */

mapping objdb;		/* ([ index : obj ]) */
mapping patchdb;	/* ([ index : patchers ]) */

static void nuke_object(object obj)
{
	destruct_object(obj);
}

/* obsolete */
private void queue_patches(int oindex, string *patches)
{
	string *old;

	if (!patch) {
		patch = clone_object(BIGSTRUCT_ARRAY_OBJ);
		patch->claim();
		patch->set_size(status(ST_OTABSIZE));
	}

	old = patch->query_element(oindex);

	if (!old) {
		old = ({ });
	}

	patch->set_element(oindex, old | patches);
}

/* obsolete */
void patch_tick(string path, int oindex, string *patches, varargs mixed *junk...)
{
	object obj;

	ACCESS_CHECK(SYSTEM());

	obj = find_object(path + "#" + oindex);

	if (obj) {
		queue_patches(oindex, patches);
		call_touch(obj);
		TOUCHD->queue_object(obj);
	}

	oindex--;

	if (oindex >= 0) {
		SUSPENDD->queue_work("patch_tick", path, oindex, patches);
	} else {
		LOGD->post_message("debug", LOG_DEBUG,
			"Finished queuing patches for clones of " + path);
	}
}

/* hooks */

/* obsolete */
string *query_patches(int oindex)
{
	if (patch) {
		return patch->query_element(oindex);
	}
}

/* obsolete */
void clear_patches(int oindex)
{
	ACCESS_CHECK(SYSTEM());

	if (patch) {
		patch->set_element(oindex, nil);
	}
}

/* obsolete */
void add_patches(string path, string *patches)
{
	int oindex;

	ACCESS_CHECK(SYSTEM());

	SUSPENDD->suspend_system();
	SUSPENDD->queue_work("patch_tick",
		path, status(ST_OTABSIZE) - 1, patches, time());

	queue_patches(status(path, O_INDEX), patches);
	TOUCHD->queue_object(find_object(path));
}

void reboot()
{
	ACCESS_CHECK(SYSTEM());

	if (patch) {
		patch->set_size(status(ST_OTABSIZE));
	}
}

void cleanup_patch()
{
	ACCESS_CHECK(SYSTEM());

	call_out("nuke_object", 0, patch);

	patch = nil;
	patchdb = nil;
	objdb = nil;
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

	patchdb = set_multilevel_map_arr(patchdb, 3, index, patchers);

	objdb = set_multilevel_map_arr(objdb, 3, index, master);
	call_touch(master);

	rlimits(0; -1) {
		int sz;

		for (sz = status(ST_OTABSIZE); --sz >= 0; ) {
			object obj;

			if (obj = find_object(path + "#" + sz)) {
				objdb = set_multilevel_map_arr(objdb, 3, sz, obj);
				call_touch(obj);
				TOUCHD->queue_object(obj);
				touchcount++;
			}
		}
	}

	LOGD->post_message("system", LOG_INFO, "Queued " + path + " and " + touchcount + " clones for patching.");
}

string *query_patchers(object obj)
{
	object odbv;
	string path;
	int index;
	int mindex;

	ACCESS_CHECK(SYSTEM());

	path = object_name(obj);
	mindex = status(obj, O_INDEX);

	if (!sscanf(path, "%s#%d", path, index)) {
		index = mindex;
	}

	odbv = query_multilevel_map_arr(objdb, 3, index);

	if (odbv == nil) {
		return nil;
	}

	ASSERT(odbv == obj);

	return query_multilevel_map_arr(patchdb, 3, mindex);
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
}

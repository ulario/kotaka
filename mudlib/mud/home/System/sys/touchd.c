/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/log.h>
#include <kotaka/paths/kotaka.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/privilege.h>
#include <status.h>
#include <type.h>

inherit SECOND_AUTO;

object queue;
object patch;

static void create()
{
}

void call_touch(object obj)
{
	ACCESS_CHECK(SYSTEM());

	if (!queue || queue->empty()) {
		queue = new_object(BIGSTRUCT_DEQUE_LWO);
		queue->claim();
		call_out("touch_tick", 0);
	}

	queue->push_back(obj);

	::call_touch(obj);
}

private void queue_patches(object obj, string *patches)
{
	string name;
	int oindex;
	string *old;

	name = object_name(obj);

	if (!sscanf(name, "%*s#%d", oindex)) {
		oindex = status(obj, O_INDEX);
	}

	old = patch->query_element(oindex);

	if (!old) {
		old = ({ });
	}

	old |= patches;

	patch->set_element(oindex, old);
}

void touch_scan_otable(string path, int sz, string *patches)
{
	int ticks;

	ACCESS_CHECK(previous_program() == SUSPENDD);

	ticks = status(ST_TICKS);

	while (sz >= 0 && ticks - status(ST_TICKS) < 100000) {
		object obj;

		obj = find_object(path + "#" + sz);

		if (obj) {
			if (patches) {
				queue_patches(obj, patches);
			}

			call_touch(obj);
		}

		sz--;
	}

	if (sz >= 0) {
		SUSPENDD->queue_work("touch_scan_otable", path, sz, patches);
		LOGD->post_message("debug", LOG_DEBUG, "TouchD: Scanning object table, currently at slot " + sz);
	}
}

void touch_all(string path, varargs string *patches)
{
	ACCESS_CHECK(SYSTEM());

	rlimits(0; -1) {
		object obj;
		object cinfo;
		object *clones;

		obj = find_object(path);

		if (patches) {
			queue_patches(obj, patches);
		}

		call_touch(find_object(path));

		if (!sscanf(path, "%*s" + CLONABLE_SUBDIR)) {
			return;
		}

		if (sscanf(path, "%*s" + LIGHTWEIGHT_SUBDIR)) {
			return;
		}

		if (sscanf(path, "%*s" + INHERITABLE_SUBDIR)) {
			return;
		}

		cinfo = CLONED->query_clone_info(status(path, O_INDEX));

		if (!cinfo) {
			return;
		}

		clones = cinfo->query_clones();

		if (clones) {
			rlimits(0; -1) {
				int sz;

				for (sz = sizeof(clones); --sz >= 0; ) {
					object clone;
					string *old;
					int oindex;

					clone = clones[sz];
					oindex = status(clone, O_INDEX);

					if (patches) {
						queue_patches(clone, patches);
					}

					call_touch(clone);
				}
			}
		} else {
			SUSPENDD->queue_work("touch_scan_otable", path, status(ST_OTABSIZE) - 1, patches);
		}
	}
}

static void touch_tick()
{
	object obj;

	obj = queue->query_front();

	queue->pop_front();

	if (obj) {
		catch {
			if (!sscanf(object_name(obj), "/kernel/%*s")) {
				obj->_F_dummy();
			}
		}
	}

	if (!queue->empty()) {
		call_out("touch_tick", 0);
	} else {
		LOGD->post_message("debug", LOG_DEBUG, "Touch queue empty");

		patch = nil;
		queue = nil;
	}
}

void add_patches(string path, string *patches)
{
	ACCESS_CHECK(SYSTEM());

	if (!patch) {
		patch = new_object(BIGSTRUCT_MAP_LWO);
		patch->claim();
		patch->set_type(T_INT);
	}

	touch_all(path, patches);
}

string *query_patches(int oindex)
{
	if (patch) {
		return patch->query_element(oindex);
	}
}

void clear_patches(int oindex)
{
	ACCESS_CHECK(SYSTEM());

	if (patch) {
		patch->set_element(oindex, nil);
	}
}

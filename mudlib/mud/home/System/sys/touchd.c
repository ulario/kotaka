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

inherit SECOND_AUTO;

object queue;

static void create()
{
}

void call_touch(object obj)
{
	ACCESS_CHECK(SYSTEM());

	if (!queue || queue->empty()) {
		queue = new_object(BIGSTRUCT_DEQUE_LWO);
		call_out("touch_tick", 0);
	}

	queue->push_back(obj);
	::call_touch(obj);
}

private void touch_scan_otable(string path)
{
	rlimits(0; -1) {
		int sz;

		for (sz = status(ST_OTABSIZE); --sz >= 0; ) {
			object obj;

			if (sz % 10000 == 0) {
				DRIVER->message(sz + " clones left to touch.\n");
			}

			obj = find_object(path + "#" + sz);

			if (obj) {
				call_touch(obj);
			}
		}
	}
}

void touch_all_clones(string path)
{
	object cinfo;
	object *clones;

	ACCESS_CHECK(SYSTEM());

	rlimits(0; -1) {
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
			int sz;

			for (sz = sizeof(clones); --sz >= 0; ) {
				call_touch(clones[sz]);
			}
		} else {
			touch_scan_otable(path);
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
		queue = nil;
	}
}

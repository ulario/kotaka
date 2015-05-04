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
	queue = new_object(BIGSTRUCT_DEQUE_LWO);
}

void call_touch(object obj)
{
	ACCESS_CHECK(SYSTEM());

	if (queue->empty()) {
		call_out("touch_tick", 0);
	}

	queue->push_back(obj);
	::call_touch(obj);
}

void touch_upgrade(string path)
{
	int sz, i;
	object cinfo;
	object *clones;

	ACCESS_CHECK(SYSTEM());

	rlimits(0; -1) {
		call_touch(find_object(path));

		cinfo = CLONED->query_clone_info(status(path, O_INDEX));

		if (!cinfo) {
			return;
		}

		clones = cinfo->query_clones();

		if (clones) {
			int sz;
			int i;

			sz = sizeof(clones);

			for (i = 0; i < sz; i++) {
				call_touch(clones[i]);
			}
		} else {
			int sz;
			int i;

			sz = status(ST_OTABSIZE);

			for (i = 0; i < sz; i++) {
				object obj;

				obj = find_object(path + "#" + i);

				if (obj) {
					call_touch(obj);
				}
			}
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
		CHANNELD->post_message("system", "touchd", "Global touch completed");
		LOGD->post_message("system", LOG_INFO, "Global touch completed");
	}
}

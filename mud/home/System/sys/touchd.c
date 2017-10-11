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
#include <kernel/access.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/privilege.h>
#include <status.h>
#include <type.h>

inherit SECOND_AUTO;

object queue;	/* objects with outstanding upgrades needing to be bumped */

int hqueue;

void touch_object(object obj)
{
	::call_touch(obj);
}

static void nuke_object(object obj)
{
	destruct_object(obj);
}

void queue_object(object obj)
{
	ACCESS_CHECK(SYSTEM());

	if (!queue) {
		queue = clone_object(BIGSTRUCT_DEQUE_OBJ);
		queue->claim();
	}

	queue->push_back(obj);

	if (!hqueue) {
		hqueue = call_out("process", 0);
	}
}

static void process()
{
	object obj;

	hqueue = 0;

	if (queue->empty()) {
		if (queue && !sscanf(object_name(queue), "%*s#-1")) {
			call_out("nuke_object", 0, queue);
		}

		queue = nil;

		LOGD->post_message("debug", LOG_DEBUG, "Finished patching");
		PATCHD->cleanup_patch();

		return;
	}

	obj = queue->query_front();
	queue->pop_front();

	hqueue = call_out("process", 0);

	if (obj) {
		obj->_F_dummy();
	}
}

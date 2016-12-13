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
object patch;	/* ([ oindex: patch functions ]) */

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

static void process(varargs int time)
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

/* flopover */

void reboot()
{
	ACCESS_CHECK(SYSTEM());

	PATCHD->reboot();
}

object give_patch()
{
	ACCESS_CHECK(previous_program() == PATCHD);

	if (patch) {
		patch->grant_access(find_object(PATCHD), FULL_ACCESS);

		return patch;
	}
}

void clear_patch()
{
	ACCESS_CHECK(SYSTEM());

	patch = nil;
}

string *query_patches(int oindex)
{
	return PATCHD->query_patches(oindex);
}

void clear_patches(int oindex)
{
	ACCESS_CHECK(SYSTEM());

	PATCHD->clear_patches(oindex);
}

void add_patches(string path, string *patches)
{
	ACCESS_CHECK(SYSTEM());

	PATCHD->add_patches(path, patches);
}

void patch_tick(string path, int oindex, string *patches, varargs mixed *junk...)
{
	ACCESS_CHECK(SYSTEM());

	PATCHD->takeover();
	PATCHD->patch_tick(path, oindex, patches);
}

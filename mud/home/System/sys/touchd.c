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
object flags;	/* ([ oindex: 1 = upgrade, 2 = touch */
object patch;	/* ([ oindex: patch functions ]) */

int qlen;
int hqueue;

/* Privileged bypass that allows anyone to touch any object */
void touch_object(object obj)
{
	::call_touch(obj);
}

static void nuke_object(object obj)
{
	destruct_object(obj);
}

private void queue_object(object obj)
{
	if (!queue) {
		queue = clone_object(BIGSTRUCT_DEQUE_OBJ);
		queue->claim();
	}

	queue->push_back(obj);
	qlen++;

	if (!hqueue) {
		hqueue = call_out("process", 0, time());
	}
}

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

string *query_patches(int oindex)
{
	if (patch) {
		return patch->query_element(oindex);
	}
}

/* Wipe an object's assigned patches */
void clear_patches(int oindex)
{
	ACCESS_CHECK(SYSTEM());

	if (patch) {
		patch->set_element(oindex, nil);
	}
}

/* work function for assigning patches */
/* to a freshly upgraded object and its clones */
void patch_tick(string path, int oindex, string *patches, int time)
{
	int curtime;
	object obj;

	ACCESS_CHECK(previous_program() == SUSPENDD);

	obj = find_object(path + "#" + oindex);

	if (obj) {
		queue_patches(oindex, patches);
		call_touch(obj);
		queue_object(obj);
	}

	curtime = time();

	if (time < curtime) {
		time = curtime;

		LOGD->post_message("debug", LOG_DEBUG,
			"Patch queue for " + path
			+ ", currently at slot " + oindex);
	}

	oindex--;

	if (oindex >= 0) {
		SUSPENDD->queue_work("patch_tick", path, oindex, patches, time);
	} else {
		LOGD->post_message("debug", LOG_DEBUG,
			"Finished queuing patches for clones of " + path);
	}
}

/* work function for lazy triggering of object patches */
static void process(int time)
{
	object obj;
	int curtime;

	hqueue = 0;

	if (queue->empty()) {
		if (queue && !sscanf(object_name(queue), "%*s#-1")) {
			call_out("nuke_object", 0, queue);
		}

		if (patch && !sscanf(object_name(patch), "%*s#-1")) {
			call_out("nuke_object", 0, patch);
		}

		queue = nil;
		patch = nil;
		qlen = 0;

		LOGD->post_message("debug", LOG_DEBUG, "Finished patching");

		return;
	}

	obj = queue->query_front();
	queue->pop_front();
	qlen--;

	curtime = time();

	if (time < curtime) {
		time = curtime;

		LOGD->post_message("debug", LOG_DEBUG,
			"Patch queue: " + qlen + " bumps left.");
	}

	hqueue = call_out("process", 0, time);

	if (obj) {
		obj->_F_dummy();
	}
}

/* assign a list of patches to a master object and its clones */
void add_patches(string path, string *patches)
{
	int oindex;

	ACCESS_CHECK(previous_program() == OBJECTD);

	SUSPENDD->suspend_system();
	SUSPENDD->queue_work("patch_tick",
		path, status(ST_OTABSIZE) - 1, patches, time());

	queue_patches(status(path, O_INDEX), patches);
	queue_object(find_object(path));
}

/* handle an enlarged object table after a reboot */
void reboot()
{
	ACCESS_CHECK(previous_program() == INITD);

	if (patch) {
		patch->set_size(status(ST_OTABSIZE));
	}
}

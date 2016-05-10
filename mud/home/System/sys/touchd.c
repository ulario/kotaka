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

void touch_object(object obj)
{
	::call_touch(obj);
}

private void queue_object(object obj)
{
	if (!queue) {
		queue = new_object(BIGSTRUCT_ARRAY_LWO);
		queue->claim();
	}

	queue->push_back(obj);
	qlen++;

	if (!hqueue) {
		hqueue = call_out("process", 0);
	}
}

private void queue_patches(int oindex, string *patches)
{
	string *old;

	if (!patch) {
		patch = new_object(BIGSTRUCT_ARRAY_LWO);
		patch->claim();
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

void clear_patches(int oindex)
{
	ACCESS_CHECK(SYSTEM());

	if (patch) {
		patch->set_element(oindex, nil);
	}
}

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
			"Queuing patches for clones of " + path
			+ ", currently at " + oindex);
	}

	oindex--;

	if (oindex >= 0) {
		SUSPENDD->queue_work("patch_tick", path, oindex, patches, time);
	} else {
		LOGD->post_message("debug", LOG_DEBUG,
			"Finished queuing patches for clones of " + path);
	}
}

static void process()
{
	object obj;

	hqueue = 0;

	if (queue->empty()) {
		queue = nil;
		patch = nil;
		qlen = 0;

		LOGD->post_message("debug", LOG_DEBUG, "Finished patching");

		return;
	}

	obj = queue->query_front();
	queue->pop_front();

	hqueue = call_out("process", 0);

	obj->_F_dummy();
}

void touch_all(string path, string *patches)
{
	int oindex;

	ACCESS_CHECK(previous_program() == OBJECTD);

	SUSPENDD->suspend_system();
	SUSPENDD->queue_work("patch_tick",
		status(ST_OTABSIZE) - 1, patches, time());

	queue_patches(status(path, O_INDEX), patches);
	queue_object(find_object(path));
}

void reboot()
{
	ACCESS_CHECK(previous_program() == INITD);

	if (patch) {
		patch->set_size(status(ST_OTABSIZE));
	}
}

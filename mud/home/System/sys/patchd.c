/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2016  Raymond Jennings
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
#include <kotaka/log.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/privilege.h>
#include <status.h>

inherit SECOND_AUTO;

object patch;

static void nuke_object(object obj)
{
	destruct_object(obj);
}

atomic void takeover()
{
	object obj;

	ACCESS_CHECK(SYSTEM());

	if (obj = TOUCHD->give_patch()) {
		TOUCHD->clear_patch();
		patch = obj;
		patch->ungrant_access(find_object(TOUCHD));
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

string *query_patches(int oindex)
{
	takeover();

	if (patch) {
		return patch->query_element(oindex);
	}
}

void clear_patches(int oindex)
{
	ACCESS_CHECK(SYSTEM());

	takeover();

	if (patch) {
		patch->set_element(oindex, nil);
	}
}

void add_patches(string path, string *patches)
{
	int oindex;

	ACCESS_CHECK(SYSTEM());

	takeover();

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
}

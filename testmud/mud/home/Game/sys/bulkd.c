/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kotaka/bigstruct.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <kotaka/paths.h>
#include <kotaka/assert.h>
#include <game/paths.h>

object queue;
int handle;

static void create()
{
	queue = clone_object(BIGSTRUCT_DEQUE_OBJ);
}

static void destruct()
{
	destruct_object(queue);
}

void reset()
{
	destruct_object(queue);

	remove_call_out(handle);

	queue = clone_object(BIGSTRUCT_DEQUE_OBJ);
}

void enqueue(object obj)
{
	ACCESS_CHECK(previous_program() == LIB_BULK);

	if (queue->empty()) {
		handle = call_out("process", 0.1, "enqueue");
	}

	queue->push_back(obj);
}

private void bulk_clean(object obj)
{
	object *inv;
	int i, sz;
	int subdirty;

	inv = obj->query_inventory();

	sz = sizeof(inv);

	for (i = 0; i < sz; i++) {
		object subobj;

		subobj = inv[i];

		if (subobj->query_bulk_dirty()) {
			subdirty = 1;

			if (!subobj->query_bulk_queued()) {
				subobj->bulk_queue();
			}
		}
	}

	if (!subdirty) {
		object env;

		obj->bulk_sync();

		env = obj->query_environment();

		if (env && env->query_bulk_dirty()) {
			if (!env->query_bulk_queued()) {
				env->bulk_queue();
			}
		}
	}
}

static void process(string who)
{
	object obj;

	handle = 0;

	if (queue->empty()) {
		error("Duplicate call by " + who);
	}

	obj = queue->get_front();
	queue->pop_front();

	obj->bulk_dequeued();

	if (obj) {
		if (obj->query_bulk_dirty()) {
			bulk_clean(obj);
		}
	}

	if (!queue->empty() && !handle) {
		LOGD->post_message("bulk", LOG_DEBUG, "Ticking...");
		handle = call_out("process", 0, "who");
	}
}

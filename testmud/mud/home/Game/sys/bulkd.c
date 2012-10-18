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

static void create()
{
	queue = clone_object(BIGSTRUCT_DEQUE_OBJ);
}

static void destruct()
{
	destruct_object(queue);
}

void enqueue(object obj)
{
	ACCESS_CHECK(previous_program() == LIB_BULK);

	LOGD->post_message("bulk", LOG_DEBUG, "Refreshing bulk cache for " + obj->query_property("id"));

	if (queue->empty()) {
		call_out("process", 0);
	}

	queue->push_back(obj);
}

static void process()
{
	object obj;

	obj = queue->get_front();
	queue->pop_front();

	if (obj) {
		object env;

		obj->bulk_sync();

		if (env = obj->query_environment()) {
			ASSERT(env->query_bulk_dirty());

			queue->push_back(env);
		}
	}

	if (!queue->empty()) {
		call_out("process", 0);
	}
}

/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <kotaka/privilege.h>
#include <game/paths.h>
#include <kotaka/bigstruct.h>

int handle;
object queue;

static void create()
{
	queue = new_object(BIGSTRUCT_DEQUE_LWO);
}

void bulk_queue(object obj)
{
	ACCESS_CHECK(previous_program() == LIB_BULK);

	queue->push_back(obj);

	if (!handle) {
		handle = call_out("process", 0);
	}
}

static void process()
{
	object obj;

	handle = 0;

	obj = queue->query_front();
	queue->pop_front();

	if (!queue->empty()) {
		handle = call_out("process", 0);
	}

	obj->bulk_sync();
}

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
#include <kotaka/paths.h>
#include <kotaka/bigstruct.h>
#include <status.h>

inherit SECOND_AUTO;

object queue;

static void create()
{
	queue = new_object(BIGSTRUCT_DEQUE_LWO);
}

void queue_touch(object obj)
{
	if (queue->empty()) {
		call_out("touch", 0);
	}

	queue->push_back(obj);
}

static void touch()
{
	object obj;

	obj = queue->get_front();

	queue->pop_front();

	if (obj) {
		obj->_F_dummy();
	}

	if (!queue->empty()) {
		call_out("touch", 0);
	}
}

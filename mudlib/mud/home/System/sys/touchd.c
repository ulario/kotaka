/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/bigstruct.h>

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
		call_out("touch", 0);
	} else {
		CHANNELD->post_message("system", "touchd", "Global touch completed");
		LOGD->post_message("system", LOG_INFO, "Global touch completed");
	}
}

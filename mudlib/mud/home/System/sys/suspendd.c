/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2015  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <kotaka/paths/bigstruct.h>

/* system suspension manager */

inherit SECOND_AUTO;

int suspend;
int handle;
object queue;

static void create()
{
}

void suspend_system()
{
	if (suspend) {
		error("System already suspended");
	}

	suspend = 1;

	if (!handle) {
		handle = call_out("process", 0);
	}

	CALLOUTD->suspend_callouts();
	SYSTEM_USERD->block_connections();
}

private void release_system()
{
	if (!suspend) {
		error("System not suspended");
	}

	CALLOUTD->release_callouts();
	SYSTEM_USERD->unblock_connections();

	suspend = 0;
}

void queue_work(string func, mixed args...)
{
	if (!handle) {
		handle = call_out("process", 0);
	}

	if (!queue) {
		queue = new_object(BIGSTRUCT_DEQUE_LWO);
		queue->claim();
	}

	queue->push_back( ({ previous_object(), func, args }) );
}

static void process()
{
	object obj;
	string func;
	mixed *args;

	handle = 0;

	catch {
		handle = call_out("process", 0);
	} : {
		if (suspend) {
			release_system();
		}
		return;
	}

	if (!queue || queue->empty()) {
		queue = nil;

		if (suspend) {
			release_system();
		}

		remove_call_out(handle);
		handle = 0;

		return;
	}

	({ obj, func, args }) = queue->query_front();
	queue->pop_front();

	if (obj) {
		call_other(obj, func, args...);
	}
}

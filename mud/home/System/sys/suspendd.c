/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <kotaka/paths/bigstruct.h>
#include <status.h>

/* system suspension manager */

inherit SECOND_AUTO;

int suspend;
int handle;
object queue;
mapping suspenders;

void release_system();

static void create()
{
}

static void destruct()
{
	if (suspend) {
		release_system();
	}
}

void suspend_system()
{
	if (suspend) {
		return;
	}

	suspend = 1;

	if (!handle) {
		handle = call_out("process", 0);
	}

	CALLOUTD->suspend_callouts();
	SYSTEM_USERD->block_connections();
}

void release_system()
{
	CALLOUTD->release_callouts();
	SYSTEM_USERD->unblock_connections();

	suspend = 0;
}

mixed *query_callouts()
{
	object obj;
	mixed **callouts;
	int sz;

	obj = previous_object();

	callouts = status(this_object(), O_CALLOUTS);

	for (sz = sizeof(callouts); --sz >= 0; ) {
		mixed *callout;

		callout = callouts[sz];

		if (sizeof(callout) <= CO_FIRSTXARG) {
			callouts[sz] = nil;
			continue;
		}

		if (callout[CO_FIRSTXARG] != obj) {
			callouts[sz] = nil;
		}
	}

	return callouts - ({ nil });
}

void queue_work(string func, mixed args...)
{
	if (!handle) {
		handle = call_out("process", 0);
	}

	if (!queue) {
		queue = clone_object(BIGSTRUCT_DEQUE_OBJ);
		queue->claim();
	}

	queue->push_front( ({ previous_object(), func, args }) );
}

int queue_delayed_work(string func, mixed delay, mixed args...)
{
	return call_out("call", delay, previous_object(), func, args);
}

mixed dequeue_delayed_work(int handle)
{
	mixed *callouts;
	int sz;

	callouts = status(this_object(), O_CALLOUTS);

	for (sz = sizeof(callouts) - 1; sz >= 0; --sz) {
		mixed *callout;

		callout = callouts[sz];

		if (callout[CO_HANDLE] != handle) {
			continue;
		}

		if (sizeof(callout) <= CO_FIRSTXARG) {
			return -1;
		}

		if (previous_object() != callout[CO_FIRSTXARG]) {
			return -1;
		}

		return remove_call_out(handle);
	}

	return -1;
}

static void call(object obj, string func, mixed *args)
{
	if (obj) {
		string owner;

		if (!suspenders) {
			suspenders = ([ ]);
		}

		owner = obj->query_owner();

		if (!suspenders[owner]) {
			if (!find_object("~/obj/suspender")) {
				compile_object("~/obj/suspender");
			}

			suspenders[owner] = clone_object("~/obj/suspender", owner);
		}

		suspenders[owner]->call(obj, func, args);
	}
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

	if (queue && queue->empty()) {
		if (!sscanf(object_name(queue), "%*s#-1")) {
			destruct_object(queue);
		} else {
			queue = nil;
		}
	}

	if (!queue) {
		if (suspend) {
			release_system();
		}

		remove_call_out(handle);
		handle = 0;

		return;
	}

	({ obj, func, args }) = queue->query_front();
	queue->pop_front();

	call(obj, func, args);
}

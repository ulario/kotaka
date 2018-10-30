/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
#include <status.h>
#include <type.h>
#include <kernel/rsrc.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

private int enough_free_callouts()
{
	int used;
	int free;
	int total;
	int quota;

	used = status(ST_NCOSHORT) + status(ST_NCOLONG);
	total = status(ST_COTABSIZE);
	free = total - used;
	quota = total / 50;

	if (quota < 10) {
		quota = 10;
	}

	return free >= quota;
}

static int find_call_out(string func)
{
	mixed **callouts;
	int sz;

	callouts = status(this_object(), O_CALLOUTS);

	for (sz = sizeof(callouts); --sz >= 0; ) {
		mixed *callout;

		callout = callouts[sz];

		if (callout[CO_FUNCTION] == func) {
			return callout[CO_HANDLE];
		}
	}

	return 0;
}

static void _F_sys_callout(string func, mixed *args)
{
	int ticks;

	ticks = status(ST_TICKS);

	rlimits (0; -1) {
		INITD->begin_task();

		catch {
			rlimits(0; ticks) {
				call_other(this_object(), func, args...);
			}
		}

		INITD->end_task();
	}
}

atomic static int call_out(string func, mixed delay, mixed args...)
{
	int handle;
	string owner;

	if (!this_object()) {
		error("Cannot call_out from destructed object");
	}

	if (!function_object(func, this_object())) {
		error("Call_out to undefined function " + func);
	}

	if (!SYSTEM() && !enough_free_callouts()) {
		error("Too many callouts");
	}

	handle = ::call_out("_F_sys_callout", delay, func, args);

	owner = query_owner();

	if (KERNELD->query_rsrc("callout peak")) {
		mixed *rsrcp, rsrcc;

		rsrcp = KERNELD->rsrc_get(owner, "callout peak");
		rsrcc = KERNELD->rsrc_get(owner, "callouts");

		if (rsrcp[RSRC_USAGE] < rsrcc[RSRC_USAGE]) {
			KERNELD->rsrc_incr(owner, "callout peak", nil, rsrcc[RSRC_USAGE] - rsrcp[RSRC_USAGE]);
		}
	}

	if (KERNELD->query_rsrc("callout usage")) {
		KERNELD->rsrc_incr(owner, "callout usage", nil, 1);
	}

	return handle;
}

private mixed **convert_callouts(mixed **callouts)
{
	int sz;

	for (sz = sizeof(callouts); --sz >= 0; ) {
		mixed *callout;

		callout = callouts[sz];

		if (callout[CO_FUNCTION] == "_F_sys_callout") {
			callouts[sz] = ({ callout[CO_HANDLE], callout[CO_FIRSTXARG], callout[CO_DELAY] }) + callout[CO_FIRSTXARG + 1];
		}
	}

	return callouts;
}

static mixed status(varargs mixed obj, mixed index)
{
	mixed status;

	switch(typeof(obj)) {
	case T_NIL:
		return ::status();

	case T_INT:
		return ::status(obj);

	case T_STRING:
	case T_OBJECT:
		switch(typeof(index)) {
		case T_NIL:
			status = ::status(obj);
			if (status) {
				status[O_CALLOUTS] = convert_callouts(status[O_CALLOUTS]);
			}
			return status;

		case T_INT:
			status = ::status(obj, index);
			if (index == O_CALLOUTS) {
				status = convert_callouts(status);
			}
			return status;

		default:
			error("Bad argument 2");
		}

	default:
		error("Bad argument 1");
	}
}

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
#include <kotaka/privilege.h>
#include <status.h>
#include <kernel/rsrc.h>
#include <kotaka/paths/system.h>

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

	handle = ::call_out(func, delay, args...);

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

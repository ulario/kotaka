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
#include <kernel/kernel.h>
#include <kotaka/paths/system.h>
#include <type.h>
#include <trace.h>

static string previous_program(varargs int n)
{
	int idx;
	string tmp;

	idx = 1;

	while (1) {
		tmp = ::previous_program(idx);

		if (tmp != CALL_GUARD) {
			if (!n) {
				return tmp;
			}
			n--;
		}
		idx++;
	}
}

static mixed **call_trace()
{
	mixed **trace;
	int index;
	int sz;

	trace = ::call_trace();
	sz = sizeof(trace) - 2; /* one for AUTO, one for us */
	trace = trace[0 .. sz - 1];

	for (index = 0; index < sz; index++) {
		mixed *frame;

		frame = trace[index];

		if (frame[TRACE_PROGNAME] == CALL_GUARD) {
			trace[index] = nil;
		}
	}

	return trace - ({ nil });
}

static object calling_object(varargs int steps)
{
	mixed **trace;

	trace = call_trace();

	return find_object(trace[sizeof(trace) - (3 + steps)][TRACE_OBJNAME]);
}

static mixed call_other(mixed obj, string func, varargs mixed args ...)
{
	object callee;
	string path;
	string fullpath;

	object this;
	string thisname;
	string thiscreator;
	string prevprog;

	prevprog = previous_program();
	this = this_object();

	if (!this) {
		error("Cannot call_other from destructed object (call made from " + prevprog + ")");
	}

	thisname = object_name(this);

	switch(typeof(obj)) {
	case T_STRING:
		path = obj;
		callee = find_object(path);
		break;

	case T_OBJECT:
		path = object_name(obj);
		callee = obj;
		break;

	default:
		error("Bad argument 1 for function call_other (type mismatch, call made from " + previous_program() + ")");
	}

	thiscreator = ::call_other(DRIVER, "creator", thisname);
	fullpath = ::call_other(DRIVER, "normalize_path", path, thisname + "/..", thiscreator);

	if (!callee) {
		error("Bad argument 1 for function call_other (target object " + fullpath + " does not exist, called by object " + thisname + " from program " + prevprog + ")");
	}

	if (!function_object(func, callee)) {
		error("Call to undefined function " + func + " in object " + fullpath + " by object " + thisname + " from program " + prevprog);
	}

	return ::call_other(callee, func, args ...);
}

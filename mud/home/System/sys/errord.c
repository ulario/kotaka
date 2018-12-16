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
#include <kotaka/log.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/string.h>
#include <kotaka/privilege.h>
#include <kotaka/assert.h>
#include <trace.h>

inherit SECOND_AUTO;
inherit "~/lib/string/align";
inherit LIB_SYSTEM;

static void create()
{
	DRIVER->set_error_manager(this_object());
}

string print_frame(mixed *frame)
{
	string str;
	mixed line;
	string func;
	string prog;
	string obj;

	line = (string)frame[TRACE_LINE];

	if (line == 0) {
		line = "";
	} else {
		line = "" + line;
	}

	func = frame[TRACE_FUNCTION];
	prog = frame[TRACE_PROGNAME];
	obj = frame[TRACE_OBJNAME];

	str = ralign(line, 5) + " " + lalign(func, 16) + " " + prog;

	if (prog != obj) {
		string path;
		int index;

		if (sscanf(obj, "%s#%d", path, index)) {
			if (prog == path) {
				str += " (#" + index + ")";
			} else {
				str += " (" + path + "#" + index + ")";
			}
		} else {
			str += " (" + obj + ")";
		}
	}

	return str;
}

string print_stack(mixed **trace, varargs int raw)
{
	int i;
	string tracestr;

	tracestr = "";

	for (i = 0; i < sizeof(trace); i++) {
		mixed *frame;

		frame = trace[i];

		if (frame[TRACE_PROGNAME] == CALL_GUARD && !raw) {
		}

		tracestr += print_frame(trace[i]) + "\n";
	}

	return tracestr;
}

private void handle_error(string error, mixed **trace)
{
	mixed **comperrs;

	TLSD->set_tls_value("System", "error-string", error);
	TLSD->set_tls_value("System", "error-trace", trace);

	comperrs = TLSD->query_tls_value("System", "compile-errors");

	if (comperrs) {
		int sz, i;

		sz = sizeof(comperrs);

		for (i = 0; i < sz; i++) {
			mixed *comperr;

			comperr = comperrs[i];

			LOGD->post_message("compile", LOG_INFO, comperr[0] + ", " + comperr[1] + ": " + comperr[2]);
		}
	}

	LOGD->post_message("error", LOG_ERR, error);
	LOGD->post_message("trace", LOG_INFO, print_stack(trace));
}

private mixed **decode_trace(string buffer)
{
	string **trace;
	string *lines;
	int sz;

	lines = explode(buffer, "\000\000");
	sz = sizeof(lines);
	trace = allocate(sz);

	for (; --sz >= 0; ) {
		string objname;
		string progname;
		string func;
		int line;

		sscanf(lines[sz], "%s\000%s\000%s\000%d", objname, progname, func, line);

		trace[sz] = ({ objname, progname, func, line });
	}

	return trace;
}

private mixed **decode_comperrs(string buffer)
{
	string **comperrs;
	string *lines;
	int sz;

	lines = explode(buffer, "\000\000");
	sz = sizeof(lines);
	comperrs = allocate(sz);

	for (; --sz >= 0; ) {
		string progname;
		int line;
		string err;

		sscanf(lines[sz], "%s\000%d\000%s", progname, line, err);

		comperrs[sz] = ({ progname, line, err });
	}

	return comperrs;
}

/*

encoded error format:

	original error
	null

	frames, separated by 2 nulls

	3 nulls

	compile errors, separated by 2 nulls

frame:

	object name
	null
	program name
	null
	function
	null
	line number

*/

void runtime_error(string error, int caught, mixed **trace)
{
	string buffer;

	ACCESS_CHECK(previous_program() == DRIVER);

	DRIVER->message("Runtime error\n" + error + "\n");

	if (sscanf(error, "%*s\000%s", buffer)) {
		string compbuf;

		DRIVER->message("Detected encoded buffer in runtime_error\n");

		sscanf(buffer, "%s\000%s", error, buffer);

		sscanf(buffer, "%s\000\000\000%s", buffer, compbuf);

		trace = decode_trace(buffer);

		if (compbuf) {
			mixed **comperrs;

			comperrs = decode_comperrs(compbuf);

			TLSD->set_tls_value("System", "compile-errors", comperrs);
		}
	} else {
		if (caught) {
			error += " [caught at " + caught + "]";
		}
	}

	error = catch(handle_error(error, trace));

	if (error) {
		DRIVER->message("Error in ErrorD::runtime_error: " + error + "\n");
	}
}

string atomic_error(string error, int atom, mixed **trace)
{
	string buffer;
	string err;

	ACCESS_CHECK(previous_program() == DRIVER);

	DRIVER->message("Atomic error\n" + error + "\n");

	if (atom) {
		error += " [atomic at " + atom + "]";
	}

	if (sscanf(error, "%*s\000%s", buffer)) {
		string compbuf;

		DRIVER->message("Detected encoded buffer in atomic_error\n");

		sscanf(buffer, "%s\000%s", error, buffer);

		sscanf(buffer, "%s\000\000\000%s", buffer, compbuf);

		trace = decode_trace(buffer);

		if (compbuf) {
			mixed **comperrs;

			DRIVER->message("Encoded comperr buffer\n");

			comperrs = decode_comperrs(compbuf);

			TLSD->set_tls_value("System", "compile-errors", comperrs);
		}
	} else {
		int sz;
		string *lines;
		mixed **comperrs;

		sz = sizeof(trace);

		lines = allocate(sz);

		for (; --sz >= 0; ) {
			mixed *frame;

			frame = trace[sz][TRACE_OBJNAME .. TRACE_LINE];
			frame[TRACE_LINE] += "";

			DRIVER->message("Imploding frame " + sz + "\n");

			lines[sz] = implode(frame, "\000");
		}

		buffer = error + "\000" + implode(lines, "\000\000");

		comperrs = TLSD->query_tls_value("System", "compile-errors");

		if (comperrs) {
			DRIVER->message("Encoding comperr buffer\n");

			sz = sizeof(comperrs);

			lines = allocate(sz);

			for (sz = sizeof(comperrs); --sz >= 0; ) {
				mixed *comperr;

				comperr = comperrs[sz];
				comperr[1] += "";

				lines[sz] = implode(comperr, "\000");
			}

			buffer += "\000\000\000" + implode(lines, "\000\000");
		}
	}

	err = catch(handle_error(error, trace));

	if (err) {
		DRIVER->message("Error in ErrorD::atomic_error: " + error + "\n");
	}

	if (buffer) {
		return error + "\000" + buffer;
	}
}

void compile_error(string file, int line, string err)
{
	mixed **comperr;

	ACCESS_CHECK(previous_program() == DRIVER);

	DRIVER->message(file + ", " + line + ": " + err + "\n");

	comperr = TLSD->query_tls_value("System", "compile-errors");

	if (!comperr) {
		comperr = ({ });
	}

	comperr += ({ ({ file, line, err }) });

	TLSD->set_tls_value("System", "compile-errors", comperr);

	DRIVER->message("Setting comperr list\n");
}

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

string print_stack(mixed **trace)
{
	int i;
	string tracestr;

	tracestr = "";

	for (i = 0; i < sizeof(trace) - 1; i++) {
		mixed *frame;

		frame = trace[i];

		if (frame[TRACE_PROGNAME] == CALL_GUARD) {
			continue;
		}

		tracestr += print_frame(trace[i]) + "\n";
	}

	return tracestr;
}

private void handle_error(string error, mixed **trace)
{
	int atom;
	int i;

	string **comperr;
	string *cerrstrs;

	string compstr;
	string errstr;
	string tracestr;

	DRIVER->set_error_manager(nil);

	catch {
		string prefix, tag, suffix;

		TLSD->set_tls_value("System", "error-string", error);
		TLSD->set_tls_value("System", "error-trace", trace);

		comperr = TLSD->query_tls_value("System", "compile-errors");

		if (comperr) {
			cerrstrs = allocate(sizeof(comperr));

			for (i = 0; i < sizeof(comperr); i++) {
				mixed *cframe;

				cframe = comperr[i];

				cerrstrs[i] =
					cframe[0] + ", " +
					cframe[1] + ": " + cframe[2];
			}
			comperr = nil;

			compstr = implode(cerrstrs, "\n");
		}

		errstr = "Runtime error: " + error;

		tracestr = print_stack(trace);

		if (find_object(LOGD)) {
			if (compstr) {
				LOGD->post_message("compile", LOG_ERR, compstr);
			}

			LOGD->post_message("error", LOG_ERR, errstr);

			if (!compstr) {
				LOGD->post_message("trace", LOG_INFO, "\n" + tracestr);
			}
		} else {
			if (compstr) {
				DRIVER->message(compstr + "\n");
			}

			DRIVER->message(errstr + "\n");

			if (!compstr) {
				DRIVER->message(tracestr + "\n");
			}

			if (find_object(CHANNELD)) {
				catch {
					if (compstr) {
						CHANNELD->post_message("compile", nil, compstr);
					}
				}

				catch {
					CHANNELD->post_message("error", nil, errstr);
				}

				if (!compstr) {
					catch {
						CHANNELD->post_message("trace", nil, tracestr);
					}
				}
			}
		}
	}

	DRIVER->set_error_manager(this_object());
}

void runtime_error(string error, int caught, mixed **trace)
{
	ACCESS_CHECK(previous_program() == DRIVER);

	if (caught) {
		error += " [caught]";
	}

	handle_error(error, trace);
}

string atomic_error(string error, int atom, mixed **trace)
{
	ACCESS_CHECK(previous_program() == DRIVER);

	if (atom) {
		error += " [atomic]";
	}

	handle_error(error, trace);
}

void compile_error(string file, int line, string err)
{
	string **comperr;

	ACCESS_CHECK(previous_program() == DRIVER);

	comperr = TLSD->query_tls_value("System", "compile-errors");

	if (!comperr) {
		comperr = ({ });
	}

	comperr += ({ ({ file, line, err }) });

	TLSD->set_tls_value("System", "compile-errors", comperr);
}

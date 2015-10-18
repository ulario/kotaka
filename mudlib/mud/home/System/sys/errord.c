/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/paths/kotaka.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <trace.h>

inherit SECOND_AUTO;

/* in atomic error, we throw a stringified mapping */
/*
([
	"comperr" : list of compilation errors,
	"atom" : frame at which code became atomic
 	"errstr" : error message
	"trace" : full stack trace
])
*/

static void create()
{
}

void disable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	DRIVER->set_error_manager(nil);
}

void enable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	DRIVER->set_error_manager(this_object());
}

/* Shamelessly stolen from Dworkin's Klib */
string sprintframe(mixed *frame, varargs int include_args)
{
	string obj;
	string prog;
	string func;
	string lineno;
	int lnum;
	int ext;
	int len;
	string line;

	string flags;

	/* GE */
	/* G = Guard */
	/* E = External */
	flags = "  ";

	obj = frame[TRACE_OBJNAME];
	prog = frame[TRACE_PROGNAME];
	func = frame[TRACE_FUNCTION];
	lnum = frame[TRACE_LINE];
	ext = frame[TRACE_EXTERNAL];

	if (ext) {
		flags[1] = 'E';
	}

	if (lnum == 0) {
		lineno = "    ";
	} else {
		lineno = "    " + lnum;
		lineno = lineno[strlen(lineno) - 4 ..];
	}

	len = strlen(func);

	if (len < 15) {
		func += "               "[len..];
	}

	if (prog != obj) {
		len = strlen(prog);
		if (len < strlen(obj) && prog == obj[..len - 1] &&
			obj[len] == '#') {
			obj = obj[len..];
		}
		line = flags + " " + lineno + " " + func + " " + prog + " (" + obj + ")";
	} else {
		line = flags + " " + lineno + " " + func + " " + prog;
	}

	if (include_args) {
		line += "\n" + STRINGD->mixed_sprint(frame[TRACE_FIRSTARG ..]);
	}

	return line;
}

string printstack(mixed **trace, varargs int include_args)
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

		tracestr += sprintframe(trace[i], include_args) + "\n";
	}

	return tracestr;
}

void runtime_error(string error, int caught, mixed **trace)
{
	int atom;
	int i;

	string **comperr;
	string *cerrstrs;

	string compstr;
	string errstr;
	string tracestr;

	ACCESS_CHECK(previous_program() == DRIVER);

	DRIVER->set_error_manager(nil);

	catch {
		if (error[0] == '(') {
			mapping thrown;
			/* gift package thrown by atomic error */

			thrown = PARSER_VALUE->parse(error);

			error = thrown["errstr"];
			atom = thrown["atom"];
			tracestr = thrown["tracestr"];
			comperr = thrown["comperr"];
		} else {
			comperr = TLSD->query_tls_value("System", "compile-errors");
			atom = -1;
		}

		TLSD->set_tls_value("System", "compile-errors", nil);

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

		if (caught) {
			errstr += " [caught]";
		}

		if (atom >= 0) {
			errstr += " [atomic at " + atom + "]";
		}

		if (!tracestr) {
			tracestr = printstack(trace);
		}

		if (find_object(LOGD)) {
			if (compstr) {
				LOGD->post_message("compile", caught ? LOG_NOTICE : LOG_ERR, compstr);
			}

			LOGD->post_message("error", caught ? LOG_NOTICE : LOG_ERR, errstr);

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

void atomic_error(string error, int atom, mixed **trace)
{
	string throwstr;
	mapping throwme;

	ACCESS_CHECK(previous_program() == DRIVER);

	throwme = ([ ]);

	throwme["atom"] = atom;
	throwme["comperr"] = TLSD->query_tls_value("System", "compile-errors");
	throwme["errstr"] = error;
	throwme["tracestr"] = printstack(trace);

	throwstr = STRINGD->mixed_sprint(throwme);

	error(throwstr);
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

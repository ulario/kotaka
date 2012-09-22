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
#include <kernel/kernel.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

#include <trace.h>
#include <type.h>

inherit SECOND_AUTO;

mixed **comperr;
string lasterror;	/*< last error */
mixed **lasttrace;	/*< last trace */
int in_errord;		/*< Prevents recursion */

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

void enable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());
	DRIVER->set_error_manager(this_object());
}

void disable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());
	DRIVER->set_error_manager(nil);
}

/** Formats a single stack frame

Shamelessly stolen from Dworkin's Klib
*/
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
	
	if (prog == CALL_GUARD) {
		flags[0] = 'G';
	}
	
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

		tracestr += sprintframe(trace[i], include_args) + "\n";
	}

	return tracestr;
}

/** Intercepts runtime errors */
void runtime_error(string error, int caught, mixed **trace)
{
	int atom;
	int i;

	string *cerrstrs;

	string compstr;
	string errstr;
	string tracestr;

	ACCESS_CHECK(previous_program() == DRIVER);

	catch {
		if (error[0] == '(') {
			mapping thrown;
			/* gift package thrown by atomic error */

			thrown = "~Kotaka/sys/parse/value"->parse(error);

			error = thrown["errstr"];
			atom = thrown["atom"];
			tracestr = thrown["tracestr"];
			comperr = thrown["comperr"];
		} else {
			atom = -1;
		}

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

		if (compstr) {
			LOGD->post_message("compile", LOG_INFO, compstr);
		}

		LOGD->post_message("error", caught ? LOG_NOTICE : LOG_ERR, errstr);
		LOGD->post_message("trace", LOG_INFO, "\n" + tracestr);
	} : {
		disable();

		LOGD->post_message("system", LOG_CRIT, "Error in error manager, error manager disabled.");
	}
}

/** Intercepts atomic errors

These are handled by packing up all our information and then
throwing a packaged error string, which runtime_error unpacks.

Um...but how the hell is this going to work if the atomic frame is UNDER
a catch frame?
*/
void atomic_error(string error, int atom, mixed **trace)
{
	string throwstr;
	mapping throwme;

	ACCESS_CHECK(previous_program() == DRIVER);

	throwme = ([ ]);

	throwme["atom"] = atom;
	throwme["comperr"] = comperr;
	throwme["errstr"] = error;
	throwme["tracestr"] = printstack(trace);

	throwstr = STRINGD->mixed_sprint(throwme);

	error(throwstr);
}

/** Records compilation errors */
void compile_error(string file, int line, string err)
{
	ACCESS_CHECK(previous_program() == DRIVER);

	if (!comperr) {
		comperr = ({ });
	}

	comperr += ({ ({ file, line, err }) });
}

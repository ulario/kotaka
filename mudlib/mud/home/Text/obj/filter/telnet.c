/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014  Raymond Jennings
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
#include <kernel/user.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>

inherit LIB_FILTER;

string inbuf;		/* raw input buffer */
string linebuf;		/* processed input waiting to be line-carved */

int echo_pending;	/* we are waiting for a DO ECHO from the client */
int echo_status;	/* echo is enabled */

static void create(int clone)
{
	if (clone) {
		::create();

		inbuf = "";
		linebuf = "";
	}
}

private void send_will(int code)
{
	string out;
	out = "\377\373 ";
	out[2] = code;
	::message(out);
}

private void send_wont(int code)
{
	string out;
	out = "\377\374 ";
	out[2] = code;
	::message(out);
}

private void send_do(int code)
{
	string out;
	out = "\377\375 ";
	out[2] = code;
	::message(out);
}

private void send_dont(int code)
{
	string out;
	out = "\377\376 ";
	out[2] = code;
	::message(out);
}

private void process_do(int code)
{
	if (code == 1) {
		if (echo_status) {
			/* already "echoing", ignore */
		} else if (echo_pending) {
			/* acknowledging our own "will echo" */
			echo_pending = 0;
			echo_status = 1;
		} else {
			/* client can't tell us what to do */
			send_wont(code);
		}
	} else {
		send_wont(code);
	}
}

private void process_dont(int code)
{
	/* honoring a dont is mandatory */
	/* however, for security reasons, we cannot be silent about it */
	if (code == 1) {
		if (echo_status) {
			::message("Warning: Your client revoked echo off\r\n");
		} else if (echo_pending) {
			::message("Warning: Your client denied echo off\r\n");
		}
		echo_status = 0;
		echo_pending = 0;
	}
	send_wont(code);
}

private void process_will(int code)
{
	/* until we understand client side options, refuse to allow them */
	send_dont(code);
}

private void process_wont(int code)
{
	/* acknowledge all of the client's wont's */
	send_dont(code);
}

int receive_message(string str)
{
	string line;

	if (str) {
		inbuf += str;
	} else {
		set_mode(::receive_message(nil));
	}

	while (strlen(inbuf)) {
		string prefix, suffix;

		if (sscanf(inbuf, "%s\377%s", prefix, suffix)) {
			linebuf += prefix;
			switch (suffix[0]) {
			case 251: /* WILL */
				inbuf = suffix[2..];
				process_will(suffix[1]);
				break;
			case 252: /* WONT */
				inbuf = suffix[2..];
				process_wont(suffix[1]);
				break;
			case 253: /* DO */
				inbuf = suffix[2..];
				process_do(suffix[1]);
				break;
			case 254: /* DONT */
				inbuf = suffix[2..];
				process_dont(suffix[1]);
				break;
			case 255: /* IAC */
				inbuf = suffix[1..];
				linebuf += "\377";
				break;
			default:
				::message
					("Unknown telnet command: "
					+ suffix[0] + "\n");
				inbuf = suffix[1..];
				break;
			}
		} else if (sscanf(inbuf, "%*s\377")) {
			/* incomplete command, wait */
			break;
		} else {
			linebuf = inbuf;
			inbuf = "";
		}

		while (sscanf(linebuf, "%s\r\n%s", line, linebuf)
			|| sscanf(linebuf, "%s\n%s", line, linebuf)) {

			set_mode(::receive_message(line));

			if (!this_object()) {
				return MODE_NOCHANGE;
			}
		}
	}

	return MODE_NOCHANGE;
}

int message(string str)
{
	str = STRINGD->replace(str, "\377", "\377\377");
	str = STRINGD->replace(str, "\n", "\r\n");
	return ::message(str);
}

void set_mode(int newmode)
{
	if (!query_conn()) {
		return;
	}

	switch (newmode) {
	case MODE_ECHO:
		if (echo_status || echo_pending) {
			send_wont(1);
		}
		echo_status = 0;
		echo_pending = 0;
		break;
	case MODE_NOECHO:
		echo_pending = 1;
		send_will(1);
		break;
	case MODE_BLOCK:
	case MODE_UNBLOCK:
	case MODE_DISCONNECT:
		::set_mode(newmode);
	case MODE_NOCHANGE:
		return;
	default:
		::message("Unknown mode change: " + newmode + "\n");
		break;
	}
}

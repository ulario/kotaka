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
#include <kernel/user.h>
#include <kotaka/paths/ansi.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/telnet.h>
#include <kotaka/log.h>

inherit LIB_FILTER;
inherit "~/lib/logging";

string inbuf;		/* raw input buffer */
string linebuf;		/* processed input waiting to be line-carved */

int echo_pending;	/* we are waiting for a DO ECHO from the client */
int echo_enabled;	/* echo is enabled */

int subcode;		/* subneg. code */
string subbuf;		/* subneg. buffer */

int naws_pending;
int naws_active;
int naws_w;
int naws_h;

private int debug;	/* debug flags: 1 = log to debug channel, 2 = echo it back */

static void create(int clone)
{
	if (clone) {
		::create();

		inbuf = "";
		linebuf = "";

		subcode = -1;
	}
}

int query_naws_pending()
{
	return naws_pending;
}

int query_naws_active()
{
	return naws_active;
}

int query_naws_width()
{
	return naws_w;
}

int query_naws_height()
{
	return naws_h;
}

int query_telnet_debug()
{
	return debug;
}

void set_telnet_debug(int new_debug)
{
	debug = new_debug;
}

private string ipof(object user)
{
	while (user && user <- LIB_USER) {
		user = user->query_conn();
	}

	if (user) {
		return query_ip_number(user);
	}
}

void send_will(int code)
{
	string out;

	out = "   ";
	out[0] = TELNET_IAC;
	out[1] = TELNET_WILL;
	out[2] = code;

	::message(out);

	log_message_out(whoami(), code + "\n", "TELNET WILL");

	if (debug & 1) {
		LOGD->post_message("debug", LOG_DEBUG, "Sent IAC WILL " + code + " to " + ipof(this_user()));
	}

	if (debug & 2) {
		::message("Sent IAC WILL " + code + "\r\n");
	}

	switch(code)
	{
	case 1:
		if (echo_enabled) {
			return;
		}
		echo_pending = 1;
		break;
	}
}

void send_wont(int code)
{
	string out;

	out = "   ";
	out[0] = TELNET_IAC;
	out[1] = TELNET_WONT;
	out[2] = code;

	::message(out);

	log_message_out(whoami(), code + "\n", "TELNET WONT");

	if (debug & 1) {
		LOGD->post_message("debug", LOG_DEBUG, "Sent IAC WONT " + code + " to " + ipof(this_user()));
	}

	if (debug & 2) {
		::message("Sent IAC WONT " + code + "\r\n");
	}

	switch(code)
	{
	case 1:
		echo_enabled = 0;
		echo_pending = 0;
		break;
	}
}

void send_do(int code)
{
	string out;

	out = "   ";
	out[0] = TELNET_IAC;
	out[1] = TELNET_DO;
	out[2] = code;

	::message(out);

	log_message_out(whoami(), code + "\n", "TELNET DO");

	if (debug & 1) {
		LOGD->post_message("debug", LOG_DEBUG, "Sent IAC DO " + code + " to " + ipof(this_user()));
	}

	if (debug & 2) {
		::message("Sent IAC DO " + code + "\r\n");
	}

	switch(code) {
	case 31:
		naws_pending = 1;
		break;
	}
}

void send_dont(int code)
{
	string out;

	out = "   ";
	out[0] = TELNET_IAC;
	out[1] = TELNET_DONT;
	out[2] = code;

	::message(out);

	log_message_out(whoami(), code + "\n", "TELNET DONT");

	if (debug & 1) {
		LOGD->post_message("debug", LOG_DEBUG, "Sent IAC DONT " + code + " to " + ipof(this_user()));
	}

	if (debug & 2) {
		::message("Sent IAC DONT " + code + "\r\n");
	}

	switch(code) {
	case 31:
		naws_pending = 0;
		naws_active = 0;
		break;
	}
}

void send_subnegotiation(int code, string subnegotiation)
{
	string out;
	out = "   ";

	out[0] = TELNET_IAC;
	out[1] = TELNET_SB;
	out[2] = code;

	::message(out);
	message(subnegotiation);

	out = "  ";
	out[0] = TELNET_IAC;
	out[1] = TELNET_SE;

	::message(out);

	log_message_out(whoami(), code + "\n", "TELNET SUBNEGOTIATION");

	if (debug & 1) {
		LOGD->post_message("debug", LOG_DEBUG, "Sent IAC SB " + code + " and SE to " + ipof(this_user()));
	}

	if (debug & 2) {
		::message("Sent IAC SB " + code + " and SE\r\n");
	}
}

private void do_subnegotiation()
{
	switch(subcode) {
	case 31:
		naws_active = 1;
		naws_w = subbuf[1] + (subbuf[0] << 8);
		naws_h = subbuf[3] + (subbuf[2] << 8);
		break;

	default:
		if (function_object("telnet_subnegotiation", query_user())) {
			query_user()->telnet_subnegotiation(subcode, subbuf);
		} else {
			int i, sz;

			::message("Your client subnegotiated for unknown option " + subcode + ":\r\n");

			for (i = 0, sz = strlen(subbuf); i < sz; i++) {
				::message(subbuf[i] + "\r\n");
			}
		}
		break;
	}
}

private void process_do(int code)
{
	if (debug & 1) {
		LOGD->post_message("debug", LOG_DEBUG, "Received IAC DO " + code + " from " + ipof(this_user()));
	}

	if (debug & 2) {
		::message("Received IAC DO " + code + "\r\n");
	}

	log_message_in(whoami(), code + "\n", "TELNET DO");

	switch(code) {
	case 1:
		if (echo_enabled) {
			/* already "echoing", ignore */
		} else if (echo_pending) {
			/* acknowledging our own "will echo" */
			echo_pending = 0;
			echo_enabled = 1;
		} else {
			/* client can't tell us what to do */
			send_wont(code);
		}
		break;

	default:
		if (function_object("telnet_do", query_user())) {
			query_user()->telnet_do(code);
		} else {
			::message("Error: client requested unknown telnet option " + code + ", refusing.\r\n");
			send_wont(code);
		}
	}
}

private void process_dont(int code)
{
	if (debug & 1) {
		LOGD->post_message("debug", LOG_DEBUG, "Received IAC DONT " + code + " from " + ipof(this_user()));
	}

	if (debug & 2) {
		::message("Received IAC DONT " + code + "\r\n");
	}

	log_message_in(whoami(), code + "\n", "TELNET DONT");

	switch(code) {
	case 1:
		/* honoring a dont is mandatory */
		/* however, for security reasons, we cannot be silent about it */
		if (echo_enabled) {
			::message("Warning: Your client revoked echo off\r\n");
		} else if (echo_pending) {
			::message("Warning: Your client denied echo off\r\n");
		} else {
			/* um...we aren't echoing in the first place? */
			break;
		}
		send_wont(code);
		break;

	default:
		if (function_object("telnet_dont", query_user())) {
			query_user()->telnet_dont(code);
		}
	}
}

private void process_will(int code)
{
	if (debug & 1) {
		LOGD->post_message("debug", LOG_DEBUG, "Received IAC WILL " + code + " from " + ipof(this_user()));
	}

	if (debug & 2) {
		::message("Received IAC WILL " + code + "\r\n");
	}

	log_message_in(whoami(), code + "\n", "TELNET WILL");

	switch(code) {
	case 31:
		if (naws_active) {
			/* ignore it, we're already doing naws */
		} else if (naws_pending) {
			/* our request was accepted */
			naws_pending = 0;
			naws_active = 1;
		} else {
			/* client is offering, accept */
			naws_active = 1;
			send_do(code);
		}
		break;

	default:
		if (function_object("telnet_will", query_user())) {
			query_user()->telnet_will(code);
		} else {
			::message("Error: client offered unknown telnet option " + code + ", forbidding\r\n");
			send_dont(code);
		}
	}
}

private void process_wont(int code)
{
	if (debug & 1) {
		LOGD->post_message("debug", LOG_DEBUG, "Received IAC WONT " + code + " from " + ipof(this_user()));
	}

	if (debug & 2) {
		::message("Received IAC WONT " + code + "\r\n");
	}

	log_message_in(whoami(), code + "\n", "TELNET WONT");

	switch(code) {
	case 31:
		/* client refused to allow or continue with NAWS */
		if (naws_active) {
			send_dont(code);
		} else if (naws_pending) {
			naws_pending = 0;
		} else {
			/* huh? */
		}
		break;

	default:
		if (function_object("telnet_wont", query_user())) {
			query_user()->telnet_wont(code);
		}
	}
}

private void process_sb(int code)
{
	if (debug & 1) {
		LOGD->post_message("debug", LOG_DEBUG, "Received IAC SB " + code + " from " + ipof(this_user()));
	}

	if (debug & 1) {
		::message("Received IAC SB " + code + "\r\n");
	}

	log_message_in(whoami(), code + "\n", "TELNET SB");

	subcode = code;
	subbuf = "";
}

private void process_se()
{
	if (debug & 1) {
		LOGD->post_message("debug", LOG_DEBUG, "Received IAC SE from " + ipof(this_user()));
	}

	if (debug & 2) {
		::message("Received IAC SE\r\n");
	}

	log_message_in(whoami(), "\n", "TELNET SE");

	do_subnegotiation();

	subcode = -1;
	subbuf = nil;
}

int login(string str)
{
	int retval;

	retval = ::login(str);

	if (retval == MODE_DISCONNECT) {
		return MODE_DISCONNECT;
	}

	send_do(31); /* attempt NAWS */

	return retval;
}

int receive_message(string str)
{
	string line;
	int pushback;

	if (str) {
		inbuf += str;
	}

	while (strlen(inbuf) && !pushback) {
		string prefix, suffix;

		if (sscanf(inbuf, "%s\377%s", prefix, suffix)) {
			if (subcode == -1) {
				linebuf += prefix;
			} else {
				subbuf += prefix;
			}

			if (!strlen(suffix)) {
				/* incomplete command */
				inbuf = " ";
				inbuf[0] = 255;
				inbuf += suffix;
				break;
			}

			switch (suffix[0]) {
			case TELNET_SB:
			case TELNET_WILL:
			case TELNET_WONT:
			case TELNET_DO:
			case TELNET_DONT:
				if (strlen(suffix) < 2) {
					/* incomplete command */
					inbuf = " ";
					inbuf[0] = 255;
					inbuf += suffix;
					pushback = TRUE;
				}
				break;
			}

			if (pushback) {
				break;
			}

			switch (suffix[0]) {
			case TELNET_WILL:
				inbuf = suffix[2 ..];
				process_will(suffix[1]);
				break;

			case TELNET_WONT:
				inbuf = suffix[2 ..];
				process_wont(suffix[1]);
				break;

			case TELNET_DO:
				inbuf = suffix[2 ..];
				process_do(suffix[1]);
				break;

			case TELNET_DONT:
				inbuf = suffix[2 ..];
				process_dont(suffix[1]);
				break;

			case TELNET_SB:
				inbuf = suffix[2 ..];
				process_sb(suffix[1]);
				break;

			case TELNET_SE:
				inbuf = suffix[1 ..];
				process_se();
				break;

			case TELNET_IAC: /* escaped IAC means a literal 255 */
				inbuf = suffix[1 ..];
				if (subcode == -1) {
					linebuf += "\377";
				} else {
					subbuf += "\377";
				}
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
		if (echo_enabled || echo_pending) {
			send_wont(1);
		}
		break;
	case MODE_NOECHO:
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

void send_raw(string text)
{
	::message(text);
}

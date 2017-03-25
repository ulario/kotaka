/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014, 2015  Raymond Jennings
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

inherit LIB_FILTER;

string inbuf;		/* raw input buffer */
string linebuf;		/* processed input waiting to be line-carved */

int echo_pending;	/* we are waiting for a DO ECHO from the client */
int echo_status;	/* echo is enabled */

int subcode;		/* subneg. code */
string subbuf;		/* subneg. buffer */

int naws;
int naws_w;
int naws_h;

static void create(int clone)
{
	if (clone) {
		::create();

		inbuf = "";
		linebuf = "";

		subcode = -1;
	}
}

int query_naws_active()
{
	return naws;
}

int query_naws_width()
{
	return naws_w;
}

int query_naws_height()
{
	return naws_h;
}

void send_will(int code)
{
	string out;
	out = "   ";
	out[0] = TELNET_IAC;
	out[1] = TELNET_WILL;
	out[2] = code;
	::message(out);
}

void send_wont(int code)
{
	string out;
	out = "   ";
	out[0] = TELNET_IAC;
	out[1] = TELNET_WONT;
	out[2] = code;
	::message(out);
}

void send_do(int code)
{
	string out;
	out = "   ";
	out[0] = TELNET_IAC;
	out[1] = TELNET_DO;
	out[2] = code;
	::message(out);
}

void send_dont(int code)
{
	string out;
	out = "   ";
	out[0] = TELNET_IAC;
	out[1] = TELNET_DONT;
	out[2] = code;
	::message(out);
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
}

private void draw_nawsbox(int w, int h)
{
	object paint;
	object gc;
	int x, y;
	string banner;

	paint = new_object(LWO_PAINTER);
	paint->set_size(w, h - 1);
	paint->add_layer("default");
	paint->set_layer_size("default", w, h - 1);

	gc = paint->create_gc();
	gc->set_layer("default");
	gc->set_clip(0, 0, w - 1, h - 2);

	gc->set_color(0x4f);

	for (y = 0; y < h; y++) {
		gc->move_pen(0, y);
		gc->draw(STRINGD->spaces(w));
		gc->move_pen(0, y);
		gc->draw("*");
		gc->move_pen(w - 1, y);
		gc->draw("*");
	}

	gc->move_pen(0, 0);
	gc->draw(STRINGD->chars('*', w));
	gc->move_pen(0, h - 2);
	gc->draw(STRINGD->chars('*', w));

	banner = "NAWS test for " + w + " columns by " + h + " rows";

	x = (w - strlen(banner)) / 2;
	y = (h - 1) / 2;

	gc->move_pen(x, y);
	gc->draw(banner);

	::message("\033[1;1H");
	::message(implode(paint->render_color(), "\n"));
}

private void do_subnegotiation()
{
	switch(subcode) {
	case 31:
		{
			naws = 1;
			naws_w = subbuf[1] + (subbuf[0] << 8);
			naws_h = subbuf[3] + (subbuf[2] << 8);
		}
		break;

	default:
		{
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

private void process_sb(int code)
{
	subcode = code;
	subbuf = "";
}

private void process_se()
{
	do_subnegotiation();

	subcode = -1;
	subbuf = nil;
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
			if (subcode == -1) {
				linebuf += prefix;
			} else {
				subbuf += prefix;
			}

			if (suffix[0] != TELNET_IAC && strlen(suffix) < 2) {
				/* incomplete command, push back and wait for more input */
				inbuf = " ";
				inbuf[0] = 255;
				inbuf += suffix;
				break;
			}

			switch (suffix[0]) {
			case TELNET_SB:
				inbuf = suffix[2 ..];
				process_sb(suffix[1]);
				break;

			case TELNET_SE:
				inbuf = suffix[1 ..];
				process_se();
				break;

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

void send_raw(string text)
{
	::message(text);
}

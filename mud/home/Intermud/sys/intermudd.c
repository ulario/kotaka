/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2017  Raymond Jennings
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
#include <kotaka/log.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/utility.h>
#include <kotaka/privilege.h>
#include <status.h>
#include <type.h>

/* Enable and redefine to suit */
/* Also inspect the query_banner function below to validate the information provided to the router */
#define MUDNAME "Kotaka"

/*
* *i4 at 204.209.44.3 port 8080
* *wpr at 195.242.99.94 port 8080
* *yatmim at 149.152.218.102 port 23
* *adsr at 62.49.9.82 port 9000
*/

/* i4
#define ROUTER_NAME	"*i4"
#define ROUTER_IP	"204.209.44.3"
#define ROUTER_PORT	8080
*/

/* dalet */
#define ROUTER_NAME	"*dalet"
#define ROUTER_IP	"97.107.133.86"
#define ROUTER_PORT	8787

/* wpr
#define ROUTER_NAME	"*wpr"
#define ROUTER_IP	"195.242.99.94"
#define ROUTER_PORT	8080
*/

/* yatmin
#define ROUTER_NAME	"*yatmin"
#define ROUTER_IP	"149.152.218.102"
#define ROUTER_PORT	23
*/

/* adsr
#define ROUTER_NAME	"*adsr"
#define ROUTER_IP	"62.49.9.82"
#define ROUTER_PORT	9000
*/

inherit LIB_USERD;
inherit LIB_SYSTEM_USER;

int handle;
string buffer;

string routername;

int password;

mapping muds;
mapping channels;

private void save();
private void restore();

#ifndef MUDNAME

static void create()
{
	MODULED->shutdown_module("Intermud");
	error("Intermud daemon not configured properly, please read ~Intermud/intermudd.c");
}

#else

static void create()
{
	muds = ([ ]);
	channels = ([ ]);

	call_out("connect", 0, ROUTER_IP, ROUTER_PORT);

	restore();
}

void listen_channel(string channel, int on);
private string mudmode_sprint(mixed data);

private string make_packet(mixed *data)
{
	int len;
	string str;
	string bigendian;

	str = mudmode_sprint(data);
	len = strlen(str) + 1;

	bigendian = "    ";

	bigendian[0] = len >> 24;
	bigendian[1] = len >> 16;
	bigendian[2] = len >> 8;
	bigendian[3] = len;

	return bigendian + str + "\000";
}

private string mudmode_sprint(mixed data)
{
	int iter;
	string tmp;
	mixed *arr;

	switch (typeof(data)) {
	case T_NIL:
		return "0";

	case T_STRING:
		return "\"" + STRINGD->quote_escape(data) + "\"";

	case T_INT:
		return (string)data;

	case T_FLOAT:
		/* decimal point is required */
		{
			string mantissa;
			string exponent;
			string str;

			str = (string)data;

			if (!sscanf(str, "%se%s", mantissa, exponent)) {
				mantissa = str;
				exponent = "";
			} else {
				exponent = "e" + exponent;
			}

			if (!sscanf(mantissa, "%*s.")) {
				mantissa += ".0";
			}

			return mantissa + exponent;
		}

	case T_ARRAY:
		if (sizeof(data) == 0)
			return "({})";

		tmp = "({";
		for (iter = 0; iter < sizeof(data); iter++) {
			tmp += mudmode_sprint(data[iter]) + ",";
		}
		return tmp + "})";

	case T_MAPPING:
		if (map_sizeof(data) == 0)
			return "([])";

		arr = map_indices(data);
		tmp = "([";
		for (iter = 0; iter < sizeof(arr); iter++) {
			tmp += mudmode_sprint(arr[iter]) + ":" +
				mudmode_sprint(data[arr[iter]]) + ",";
		}
		return tmp + "])";
	}
}

/* logging */

private void log_outbound(mixed *arr)
{
}

private void log_inbound(mixed *arr)
{
}

/* connection management */

string query_banner(object LIB_CONN connection)
{
	mixed *arr;

	arr = ({
		"startup-req-3",
		5,
		MUDNAME,
		0,
		ROUTER_NAME,
		0,

		password,
		0,
		0,

		50000,
		0,
		0,
		"Kotaka",
		"Kotaka",
		status(ST_VERSION),
		"DGD",
		"(fill in the purpose of the mud here)",
		"(fill in your admin email here)",
		([
			"channel":1
		]),
		([ ])
	});

	log_outbound(arr);

	return make_packet(arr);
}

int query_timeout(object LIB_CONN connection)
{
	return 3;
}

object select(string input)
{
	return this_object();
}

void send_channel_message(string channel, string sender, string text)
{
	mixed *arr;

	arr = ({
		"channel-m",
		5,
		MUDNAME,
		sender ? sender : "system",
		0,
		0,
		channel,
		sender ? STRINGD->to_title(sender) : "(system)",
		text
	});

	log_outbound(arr);

	message(make_packet(arr));
}

private void do_chanlist_reply(mixed *value)
{
	mapping delta;
	string *names;
	mixed *values;
	int sz;

	delta = value[7];

	names = map_indices(delta);
	values = map_values(delta);

	sz = sizeof(names);

	for (sz = sizeof(names) - 1; sz >= 0; --sz) {
		if (values[sz] == 0) {
			channels[names[sz]] = nil;
		} else {
			channels[names[sz]] = values[sz];

			if (CHANNELD->query_intermud(names[sz])) {
				listen_channel(names[sz], 1);
			}
		}
	}

	save();
}

private void do_emoteto(mixed *value)
{
	object user;

	if (user = TEXT_USERD->find_user(value[5])) {
		user->message(value[6] + "@" + value[2] + " emotes to you: " + value[7]);
	} else {
		mixed *arr;

		arr = ({
			"error",
			5,
			MUDNAME,
			0,
			value[2],
			value[3],
			"unk-user",
			"User not online: " + value[5],
			value
		});

		log_outbound(arr);

		message(make_packet(arr));
	}
}

private void do_mudlist(mixed *value)
{
	mapping delta;
	string *names;
	mixed *values;
	int sz;

	delta = value[7];

	names = map_indices(delta);
	values = map_values(delta);

	for (sz = sizeof(names) - 1; sz >= 0; --sz) {
		if (values[sz] == 0) {
			muds[names[sz]] = nil;
		} else {
			muds[names[sz]] = values[sz];
		}
	}

	save();
}

private void do_startup_reply(mixed *value)
{
	string *ch;
	mixed *raw;
	int sz;

	LOGD->post_message("system", LOG_NOTICE, "Intermud: Received startup reply");

	password = value[7];

	save();

	ch = CHANNELD->query_channels();
	sz = sizeof(ch);

	for (sz = sizeof(ch) - 1; sz >= 0; --sz) {
		if (channels[ch[sz]]) {
			listen_channel(ch[sz], CHANNELD->query_intermud(ch[sz]));
		}
	}
}

private void do_tell(mixed *value)
{
	object user;

	if (user = TEXT_USERD->find_user(value[5])) {
		string msg;

		msg = value[7];

		if (msg[strlen(msg) - 1] != '\n') {
			msg += "\n";
		}

		user->message(value[6] + "@" + value[2] + " tells you: " + msg);
	} else {
		mixed *arr;

		arr = ({
			"error",
			5,
			MUDNAME,
			0,
			value[2],
			value[3],
			"unk-user",
			"User not online: " + value[5],
			value
		});

		log_outbound(arr);

		message(make_packet(arr));
	}
}

private void do_channel_m(mixed *value)
{
	if (CHANNELD->test_channel(value[6])) {
		CHANNELD->post_message(value[6], value[7] + "@" + value[2], value[8], 1);
	}
}

private void do_channel_e(mixed *value)
{
	if (CHANNELD->test_channel(value[6])) {
		string text;

		text = STRINGD->replace(
			value[8], "$N", value[7] + "@" + value[2]);

		CHANNELD->post_message(value[6], text, nil, 1);
	}
}

private void bounce_packet(mixed *value)
{
	mixed *arr;

	/* send back an error packet */
	LOGD->post_message("intermud", LOG_INFO,
		"Unhandled packet:\n" + STRINGD->hybrid_sprint(value) + "\n");

	LOGD->post_message("intermud", LOG_INFO,
		"Bouncing back an error to \"" + value[2] + "\"\n");

	arr = ({
		"error",
		5,
		MUDNAME,
		0,
		value[2],
		value[3],
		"unk-type",
		"Unhandled packet type: " + value[0],
		value
	});

	log_outbound(arr);

	message(make_packet(arr));
}

private void process_packet(mixed *value)
{
	switch(value[0]) {
	case "chanlist-reply":
		do_chanlist_reply(value);
		break;

	case "channel-m":
		do_channel_m(value);
		break;

	case "channel-e":
		do_channel_e(value);
		break;

	case "emoteto":
		do_emoteto(value);
		break;

	case "error":
		LOGD->post_message("error", LOG_ERR, "I3 error: " + STRINGD->mixed_sprint(value));
		break;

	case "mudlist":
		do_mudlist(value);
		break;

	case "startup-reply":
		do_startup_reply(value);
		break;

	case "tell":
		do_tell(value);
		break;

	/* we don't care about these right now */
	case "finger-req": /* ignored for privacy */
	case "locate-req": /* ignored for privacy */
	case "ucache-update": /* we don't implement ucache */
	case "who-req": /* ignored for privacy */
		break;

	default:
		bounce_packet(value);
	}
}

static void process()
{
	mixed *arr;
	int len;
	string packet;

	handle = 0;

	if (strlen(buffer) < 4) {
		return;
	}

	len = buffer[3] + (buffer[2] << 8) + (buffer[1] << 16) + (buffer[0] << 24);

	if (strlen(buffer) < len + 4) {
		return;
	}

	buffer = buffer[4 ..];
	packet = buffer[0 .. len - 2];
	buffer = buffer[len ..];

	arr = PARSER_MUDMODE->parse(packet);

	log_inbound(arr);

	process_packet(arr);

	handle = call_out("process", 0);
}

/* communication */

int login(string input)
{
	buffer = input;

	connection(previous_object());

	handle = call_out("process", 0);

	return MODE_NOCHANGE;
}

int receive_message(string input)
{
	buffer += input;

	if (!handle) {
		handle = call_out("process", 0);
	}

	return MODE_NOCHANGE;
}

int message_done()
{
	return MODE_NOCHANGE;
}

void logout(int quit)
{
	if (!quit) {
		LOGD->post_message("intermud", LOG_NOTICE, "Intermud: Connection lost");

		call_out("connect", 0, ROUTER_IP, ROUTER_PORT);
	}

	if (handle) {
		remove_call_out(handle);
	}

	muds = ([ ]);
	channels = ([ ]);

	buffer = nil;
}

void connect_failed(int refused)
{
	LOGD->post_message("intermud", LOG_NOTICE, "Intermud: Connection failed");

	call_out("connect", 10, ROUTER_IP, ROUTER_PORT);
}

static void destruct()
{
	if (query_conn()) {
		disconnect();
	}
}

string *query_channels()
{
	return map_indices(channels);
}

mixed *query_channel(string channel)
{
	return SUBD->deep_copy(channels[channel]);
}

string *query_muds()
{
	return map_indices(muds);
}

mixed *query_mud(string mud)
{
	return SUBD->deep_copy(muds[mud]);
}

void listen_channel(string channel, int on)
{
	mixed *arr;

	ACCESS_CHECK(INTERFACE());

	arr = ({
		"channel-listen",
		5,
		MUDNAME,
		0,
		ROUTER_NAME,
		0,
		channel,
		on
	});

	log_outbound(arr);

	message(make_packet(arr));
}

void add_channel(string channel)
{
	mixed *arr;

	ACCESS_CHECK(INTERFACE());

	if (channels[channel] && channels[channel][0] != MUDNAME) {
		error("Not our channel");
	}

	channels[channel] = ({ MUDNAME, 0 });

	arr = ({
		"channel-add",
		5,
		MUDNAME,
		0,
		ROUTER_NAME,
		0,
		channel,
		0
	});

	log_outbound(arr);

	message(make_packet(arr));
}

void remove_channel(string channel)
{
	mixed *arr;

	ACCESS_CHECK(INTERFACE());

	if (!channels[channel]) {
		error("No such channel");
	}

	if (channels[channel][0] != MUDNAME) {
		error("Not our channel");
	}

	channels[channel] = nil;

	arr = ({
		"channel-remove",
		5,
		MUDNAME,
		0,
		ROUTER_NAME,
		0,
		channel
	});

	log_outbound(arr);

	message(make_packet(arr));
}

#endif

private void save()
{
	string buf;

	buf = STRINGD->hybrid_sprint( ([
		"password" : password
	]) );

	SECRETD->make_dir(".");
	SECRETD->remove_file("intermud-tmp");
	SECRETD->write_file("intermud-tmp", buf + "\n");
	SECRETD->remove_file("intermud");
	SECRETD->rename_file("intermud-tmp", "intermud");
}

private void restore()
{
	mapping map;
	string buf;

	muds = ([ ]);
	channels = ([ ]);
	password = 0;

	buf = SECRETD->read_file("intermud");

	if (buf) {
		catch {
			map = PARSER_VALUE->parse(buf);

			if (map && map["password"]) {
				password = map["password"];
			}
		} : {
			LOGD->post_message("system", LOG_ERR, "Error parsing Intermud state, resetting");
			SECRETD->remove_file("intermud-bad");
			SECRETD->rename_file("intermud", "intermud-bad");
		}
	} else {
		return;
	}
}

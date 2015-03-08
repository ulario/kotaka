/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013, 2014  Raymond Jennings
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

/* mud mode */
/* (big endian 4 byte integer, length of the string) (string) (null) */

#define ROUTER_IP	"204.209.44.3"
#define ROUTER_PORT	8080
#define MUDNAME		"Ulario"

inherit LIB_USERD;
inherit LIB_SYSTEM_USER;

int handle;
string buffer;

string routername;

int password;

int mudlistid;
int chanlistid;

mapping muds;
mapping channels;

private void save();
private void restore();

static void create()
{
	muds = ([ ]);
	channels = ([ ]);

	mudlistid = 0;
	chanlistid = 0;

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

	bigendian[0] = (len & 0xff000000) >> 24;
	bigendian[1] = (len & 0xff0000) >> 16;
	bigendian[2] = (len & 0xff00) >> 8;
	bigendian[3] = len & 0xff;

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

/* connection management */

string query_banner(object LIB_CONN connection)
{
	mixed *startup;
	string packet;

	return make_packet( ({
		"startup-req-3",
		5,
		MUDNAME,
		0,
		"*i4",
		0,

		password,
		mudlistid,
		0,

		50000,
		0,
		0,
		"Kotaka",
		"Kotaka",
		status(ST_VERSION),
		"DGD",
		"mudlib development",
		"shentino@gmail.com",
		([
			"channel":1
		]),
		([ ])
	}) );
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
	mixed *packet;

	message(make_packet( ({
		"channel-m",
		5,
		MUDNAME,
		sender ? sender : "system",
		0,
		0,
		channel,
		sender ? STRINGD->to_title(sender) : "(system)",
		text
	}) ));
}

private void do_chanlist_reply(mixed *value)
{
	mapping delta;
	string *names;
	mixed *values;
	int sz;

	chanlistid = value[6];

	delta = value[7];

	names = map_indices(delta);
	values = map_values(delta);

	sz = sizeof(names);

	for (sz = sizeof(names) - 1; sz >= 0; --sz) {
		if (values[sz] == 0) {
			channels[names[sz]] = nil;
		} else {
			channels[names[sz]] = values[sz];
		}
	}
}

private void do_emoteto(mixed *value)
{
	object user;

	if (user = TEXT_USERD->find_user(value[5])) {
		user->message(value[6] + "@" + value[2] + " emotes to you: " + value[7]);
	} else {
		message(make_packet( ({
			"error",
			5,
			MUDNAME,
			0,
			value[2],
			value[3],
			"unk-user",
			"User not online: " + value[5],
			value
		}) ));
	}
}

private void do_mudlist(mixed *value)
{
	mapping delta;
	string *names;
	mixed *values;
	int sz;

	mudlistid = value[6];

	save();

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
}

private void do_startup_reply(mixed *value)
{
	string *ch;
	mixed *raw;
	int sz;

	LOGD->post_message("intermud", LOG_INFO, "Intermud: Received startup reply");

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
		user->message(value[6] + "@" + value[2] + " tells you: " + value[7]);
	} else {
		message(make_packet( ({
			"error",
			5,
			MUDNAME,
			0,
			value[2],
			value[3],
			"unk-user",
			"User not online: " + value[5],
			value
		}) ));
	}
}

private void do_channel_m(value)
{
	if (CHANNELD->test_channel(value[6])) {
		CHANNELD->post_message(value[6], value[7] + "@" + value[2], value[8], 1);
	}
}

private void do_channel_e(value)
{
	if (CHANNELD->test_channel(value[6])) {
		CHANNELD->post_message(value[6], value[7] + "@" + value[2], value[8], 1);
	}
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
		CHANNELD->post_message("error", nil, "intermud error: " + STRINGD->mixed_sprint(value));
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

	case "ucache-update":
		/* we don't care about these right now */
		break;

	default:
		LOGD->post_message("intermud", LOG_INFO, "Unhandled packet:\n" + STRINGD->hybrid_sprint(value) + "\n");
		LOGD->post_message("intermud", LOG_INFO, "Bouncing back an error to \"" + value[2] + "\"\n");

		{
			/* send back an error packet */
			message(make_packet( ({
				"error",
				5,
				MUDNAME,
				0,
				value[2],
				value[3],
				"unk-type",
				"Unhandled packet type: " + value[0],
				value
			}) ));
		}
	}
}

static void process()
{
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

	process_packet(PARSER_MUDMODE->parse(packet));

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
		LOGD->post_message("intermud", LOG_INFO, "Intermud: Connection lost");

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
	LOGD->post_message("intermud", LOG_INFO, "Intermud: Connection failed");

	call_out("connect", 10, ROUTER_IP, ROUTER_PORT);
}

static void destruct()
{
	disconnect();
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
	ACCESS_CHECK(INTERFACE());

	message(make_packet( ({
		"channel-listen",
		5,
		MUDNAME,
		0,
		"*i4",
		0,
		channel,
		on
	}) ));
}

void add_channel(string channel)
{
	ACCESS_CHECK(INTERFACE());

	if (channels[channel] && channels[channel][0] != MUDNAME) {
		error("Not our channel");
	}

	channels[channel] = ({ MUDNAME, 0 });

	message(make_packet( ({
		"channel-add",
		5,
		MUDNAME,
		0,
		"*i4",
		0,
		channel,
		0
	}) ));
}

void remove_channel(string channel)
{
	ACCESS_CHECK(INTERFACE());

	if (!channels[channel]) {
		error("No such channel");
	}

	if (channels[channel][0] != MUDNAME) {
		error("Not our channel");
	}

	channels[channel] = nil;

	message(make_packet( ({
		"channel-remove",
		5,
		MUDNAME,
		0,
		"*i4",
		0,
		channel
	}) ));
}

void reset()
{
	disconnect();

	mudlistid = 0;
	chanlistid = 0;

	muds = ([ ]);
	channels = ([ ]);

	LOGD->post_message("intermud", LOG_INFO, "Reset");

	call_out("connect", 0, ROUTER_IP, ROUTER_PORT);
}

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

	buf = SECRETD->read_file("intermud");

	if (!buf) {
		return;
	}

	map = PARSER_VALUE->parse(buf);

	if (map["password"]) {
		password = map["password"];
	}
}

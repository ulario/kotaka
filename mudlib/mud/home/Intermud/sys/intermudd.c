/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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

/* mud mode */
/* (big endian 4 byte integer, length of the string) (string) (null) */
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kernel/user.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <text/paths.h>
#include <status.h>
#include <type.h>

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
mixed *routers;	/* ({ name, ({ ip, port }) }) */

static void create()
{
	call_out("connect", 0, "204.209.44.3", 8080);

	muds = ([ ]);
	channels = ([ ]);
}

void listen_channel(string channel, int on);

string to_packet(string data)
{
	int len;
	string bigendian;

	len = strlen(data) + 1;

	bigendian = "    ";

	bigendian[0] = (len & 0xff000000) >> 24;
	bigendian[1] = (len & 0xff0000) >> 16;
	bigendian[2] = (len & 0xff00) >> 8;
	bigendian[3] = len & 0xff;

	return bigendian + data + "\000";
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

	startup = ({
		"startup-req-3",
		5,
		"Ulario",
		0,
		"*i4",
		0,

		password,
		0,
		0,

		50000,
		mudlistid,
		chanlistid,
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
	});

	packet = to_packet(mudmode_sprint(startup));

	return packet;
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
	string packet;

	packet = mudmode_sprint(
		({
			"channel-m",
			5,
			"Ulario",
			sender ? sender : "system",
			0,
			0,
			channel,
			sender ? STRINGD->to_title(sender) : "(system)",
			text
		})
	);

	message(to_packet(packet));
}

private void process_packet(string packet)
{
	mixed *value;

	string mtype;
	int ttl;

	mixed omud;
	mixed ouser;

	mixed tmud;
	mixed tuser;

	value = PARSE_MUDMODE->parse(packet);

	({ mtype, ttl, omud, ouser, tmud, tuser }) = value[0 .. 5];

	switch(mtype) {
	case "chanlist-reply":
		chanlistid = value[6];

		{
			mapping delta;
			string *names;
			mixed *values;

			int i, sz;

			delta = value[7];

			names = map_indices(delta);
			values = map_values(delta);

			sz = sizeof(names);

			for (i = 0; i < sz; i++) {
				if (values[i] == 0) {
					channels[names[i]] = nil;
				} else {
					channels[names[i]] = values[i];
				}
			}
		}

		break;

	case "channel-m":
		if (CHANNELD->test_channel(value[6])) {
			CHANNELD->post_message(value[6], value[7] + "@" + omud, value[8], 1);
		}

		break;

	case "channel-e":
		if (CHANNELD->test_channel(value[6])) {
			CHANNELD->post_message(value[6], value[7] + "@" + omud, value[8], 1);
		}

		break;

	case "emoteto":
		{
			object user;

			if (user = TEXT_USERD->find_user(tuser)) {
				user->message(value[6] + "@" + omud + " emotes to you: " + value[7]);
			} else {
				message(to_packet(mudmode_sprint(
				({
					"error",
					5,
					"Ulario",
					0,
					omud,
					ouser,
					"unk-user",
					"User not online: " + tuser,
					value
				}) )));
			}
		}


	case "error":
		CHANNELD->post_message("error", nil, "intermud error: " + STRINGD->mixed_sprint(value));
		break;

	case "mudlist":
		mudlistid = value[6];

		{
			mapping delta;
			string *names;
			mixed *values;

			int i, sz;

			delta = value[7];

			names = map_indices(delta);
			values = map_values(delta);

			sz = sizeof(names);

			for (i = 0; i < sz; i++) {
				if (values[i] == 0) {
					muds[names[i]] = nil;
				} else {
					muds[names[i]] = values[i];
				}
			}
		}

		break;

	case "startup-reply":
		LOGD->post_message("intermud", LOG_INFO, "Received startup reply");

		password = value[7];
		routers = ({ });

		{
			mixed *raw;
			int i, sz;

			raw = value[6];
			sz = sizeof(raw);

			for (i = 0; i < sz; i++) {
				string *router;
				string name;
				string addr;
				string ip;
				int port;

				router = raw[i];
				name = router[0];
				addr = router[1];

				sscanf(addr, "%s %d", ip, port);

				routers += ({ name, ({ ip, port }) });
			}
		}

		{
			string *ch;
			int i, sz;

			ch = CHANNELD->query_channels();
			sz = sizeof(ch);

			for (i = 0; i < sz; i++) {
				if (channels[ch[i]]) {
					listen_channel(ch[i], CHANNELD->query_intermud(ch[i]));
				}
			}
		}

		break;

	case "tell":
		{
			object user;

			if (user = TEXT_USERD->find_user(tuser)) {
				user->message(value[6] + "@" + omud + " tells you: " + value[7]);
			} else {
				message(to_packet(mudmode_sprint(
				({
					"error",
					5,
					"Ulario",
					0,
					omud,
					ouser,
					"unk-user",
					"User not online: " + tuser,
					value
				}) )));
			}
		}

	default:
		LOGD->post_message("intermud", LOG_INFO, "Unhandled packet:\n" + STRINGD->hybrid_sprint(value) + "\n");
		LOGD->post_message("intermud", LOG_INFO, "Bouncing back an error to \"" + omud + "\"\n");

		{
			/* send back an error packet */
			message(to_packet(mudmode_sprint(
				({
					"error",
					5,
					"Ulario",
					0,
					omud,
					ouser,
					"unk-type",
					"Unhandled packet type: " + mtype,
					value
				})
			)));
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

	process_packet(packet);

	handle = call_out("process", 0);
}

/* communication */

int login(string input)
{
	buffer = input;

	connection(previous_object());

	write_file("i3-in", input);

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
		LOGD->post_message("intermud", LOG_INFO, "Connection lost");

		call_out("connect", 0, "204.209.44.3", 8080);
	}

	if (handle) {
		remove_call_out(handle);
	}

	buffer = nil;
}

void connect_failed(object connection)
{
	LOGD->post_message("intermud", LOG_INFO, "Connection failed");

	call_out("connect", 10, "204.209.44.3", 8080);
}

static void destruct()
{
	disconnect();
}

string *query_channels()
{
	return map_indices(channels);
}

string *query_muds()
{
	return map_indices(muds);
}

void listen_channel(string channel, int on)
{
	ACCESS_CHECK(INTERFACE());

	message(to_packet(mudmode_sprint(
	({
		"channel-listen",
		5,
		"Ulario",
		0,
		"*i4",
		0,
		channel,
		on
	})
	)));
}

void add_channel(string channel, int type)
{
	ACCESS_CHECK(INTERFACE());

	if (channels[channel] && channels[channel][0] != "Ulario") {
		error("Not our channel");
	}

	channels[channel] = ({ "Ulario", type });

	message(to_packet(mudmode_sprint(
	({
		"channel-add",
		5,
		"Ulario",
		0,
		"*i4",
		0,
		channel,
		type
	})
	)));
}

void remove_channel(string channel)
{
	ACCESS_CHECK(INTERFACE());

	if (!channels[channel]) {
		error("No such channel");
	}

	if (channels[channel][0] != "Ulario") {
		error("Not our channel");
	}

	channels[channel] = nil;

	message(to_packet(mudmode_sprint(
	({
		"channel-remove",
		5,
		"Ulario",
		0,
		"*i4",
		0,
		channel
	})
	)));
}

void reset()
{
	disconnect();

	mudlistid = 0;
	chanlistid = 0;

	muds = ([ ]);
	channels = ([ ]);

	LOGD->post_message("intermud", LOG_INFO, "Reset");

	call_out("connect", 0, "204.209.44.3", 8080);
}

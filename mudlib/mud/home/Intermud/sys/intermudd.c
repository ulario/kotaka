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
#include <kernel/user.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <status.h>
#include <type.h>

inherit LIB_USERD;
inherit LIB_SYSTEM_USER;

object routerconn;
int password;
string buffer;

static void create()
{
	call_out("connect", 0, "204.209.44.3", 8080);
}

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

private string packed_sprint(mixed data)
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
			tmp += packed_sprint(data[iter]) + ",";
		}
		return tmp + "})";

	case T_MAPPING:
		if (map_sizeof(data) == 0)
			return "([])";

		arr = map_indices(data);
		tmp = "([";
		for (iter = 0; iter < sizeof(arr); iter++) {
			tmp += packed_sprint(arr[iter]) + ":" +
				packed_sprint(data[arr[iter]]) + ",";
		}
		return tmp + "])";
	}
}

/* connection management */

string query_banner(object LIB_CONN connection)
{
	mixed *startup;
	string packet;

	ASSERT(!routerconn);

	startup = ({
		"startup-req-3",
		5,
		"Ulario",
		0,
		"*i4",
		0,

		0,
		0,
		0,

		50000,
		0,
		0,
		"Kotaka - testing",
		"kernellib",
		status(ST_VERSION),
		"DGD",
		"mudlib development",
		"shentino@gmail.com",
		([
			"channel":1
		]),
		([ ])
	});

	packet = to_packet(packed_sprint(startup));

	return packet;
}

int query_timeout(object LIB_CONN connection)
{
	ASSERT(!routerconn);

	return 3;
}

object select(string input)
{
	return this_object();
}

private void process_packet(string packet)
{
	mixed value;

	value = "~Kotaka/sys/parse/mudmode"->parse(packet);

	LOGD->post_message("intermud", LOG_INFO, "Packet:\n" + STRINGD->hybrid_sprint(value) + "\n");
}

static void process()
{
	int len;
	string packet;

	if (strlen(buffer) < 4) {
		return;
	}

	len = buffer[3] + (buffer[2] << 8) + (buffer[1] << 16) + (buffer[0] << 24);

	LOGD->post_message("intermud", LOG_INFO, "Packet size: " + len);

	if (strlen(buffer) < len + 4) {
		return;
	}

	buffer = buffer[4 ..];
	packet = buffer[0 .. len - 2];
	buffer = buffer[len ..];

	process_packet(packet);

	call_out("process", 0);
}

/* communication */

int login(string input)
{
	buffer = input;

	connection(previous_object());

	write_file("i3-in", input);

	LOGD->post_message("intermud", LOG_INFO, "Received input");

	call_out("process", 0);

	return MODE_NOCHANGE;
}

int receive_message(string input)
{
	buffer += input;

	LOGD->post_message("intermud", LOG_INFO, "Received input");

	write_file("i3-in", input);

	call_out("process", 0);

	return MODE_NOCHANGE;
}

int message_done()
{
	return MODE_NOCHANGE;
}

void logout(int quit)
{
	buffer = nil;
}

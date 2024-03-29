/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2019, 2020, 2021, 2022  Raymond Jennings
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
#include <kotaka/assert.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/ports.h>
#include <kotaka/privilege.h>
#include <status.h>
#include <type.h>

/* Enable and redefine to suit */
/* Also inspect the query_banner function below to validate the information provided to the router */

inherit LIB_USERD;
inherit LIB_SYSTEM_USER;
inherit "/lib/copy";
inherit "/lib/string/case";
inherit "/lib/string/sprint";
inherit "/lib/string/replace";
inherit "~System/lib/utility/secretlog";
inherit "~Text/lib/sub";

/* i3 information */
static mapping muds;
static mapping channels;

/* daemon state */
string buffer;
string mudname;
string base_mudname;
int rejections;
int strikes;
int pinged;
int ponged;
int active;
int legacy_active_checked;

/* i3 interface */
int password; /* deprecated */
mapping passwords;
string router;

/* local */
mapping routers;

void restore();

/* utility */

private mixed *sanitize_control(string input)
{
	int i, sz, cflag;
	string output;

	output = "";

	for (sz = strlen(input), i = 0; i < sz; i++) {
		string tip;

		if (input[i] < ' ') {
			tip = "^@";
			tip[1] += input[i];
			cflag = 1;
		} else {
			tip = " ";
			tip[0] = input[i];
		}

		output += tip;
	}

	return ({ cflag, output });
}

private void clean_passwords()
{
	int time;
	int sz;
	string *names;
	mixed *vals;

	time = time();

	if (!passwords) {
		passwords = ([ ]);
	}

	names = map_indices(passwords);
	vals = map_values(passwords);

	for (sz = sizeof(names); --sz >= 0; ) {
		int expire;
		int pass;
		string name;
		mixed val;

		name = names[sz];
		val = vals[sz];

		switch(typeof(val)) {
		case T_ARRAY:
			({ pass, expire }) = val;

			if (expire <= time) {
				passwords[name] = nil;
			}

			break;

		case T_INT:
			passwords[name] = ({ val, time + 7 * 86400 });
		}
	}

	if (password) {
		passwords[base_mudname] = ({ password, time + 7 * 86400 });
		password = 0;
	}
}

private int query_password(string name)
{
	int *arr;

	clean_passwords();

	if (arr = passwords[name]) {
		return arr[0];
	} else {
		return 0;
	}
}

private void set_password(string name, int pass)
{
	clean_passwords();

	passwords[name] = ({ pass, time() + 7 * 86400 });
}

private void clear_password(string name)
{
	clean_passwords();

	passwords[name] = nil;
}

private void reset_routers()
{
	routers = ([
		"*dalet": ({ "97.107.133.86", 8787 }),
		"*Kelly": ({ "150.101.219.57", 8080 }),
		"*wpr": ({ "136.144.155.250", 8080 })
	]);

	router = "*dalet";
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
		return "\"" + quote_escape(data) + "\"";

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

private int is_keepalive_packet(mixed *arr)
{
	return arr[0] == "tell" && arr[2] == mudname && arr[3] == 0 && arr[4] == mudname && arr[5] == 0 && arr[6] == "IntermudD Keepalive";
}

/* I3 utility */

private void i3_send_packet(mixed *arr)
{
	if (!is_keepalive_packet(arr)) {
		write_secret_log("packet-log", timestamp(millitime()) + " >>> " + mixed_sprint(arr));
	}

	message(make_packet(arr));
}

private void i3_bounce_packet(mixed *value)
{
	mixed *arr;

	/* send back an error packet */
	LOGD->post_message("system", LOG_ERR,
		"IntermudD: Unhandled packet, bouncing an error back to \"" + value[2] + "\":\n" + hybrid_sprint(value) + "\n");

	arr = ({
		"error",
		5,
		mudname,
		0,
		value[2],
		value[3],
		"unk-type",
		"Unhandled packet type: " + value[0],
		value
	});

	i3_send_packet(arr);
}

private void i3_listen_channel(string channel)
{
	mixed *arr;

	arr = ({
		"channel-listen",
		5,
		mudname,
		0,
		router,
		0,
		channel,
		1
	});

	i3_send_packet(arr);
}

private void i3_unlisten_channel(string channel)
{
	mixed *arr;

	arr = ({
		"channel-listen",
		5,
		mudname,
		0,
		router,
		0,
		channel,
		0
	});

	i3_send_packet(arr);
}

/* I3 packet handlers */

private void i3_handle_chanlist_reply(mixed *value)
{
	mapping delta;
	string *names;
	mixed *values;
	int sz;

	delta = value[7];

	names = map_indices(delta);
	values = map_values(delta);
	sz = sizeof(names);

	for (sz = sizeof(names); --sz >= 0; ) {
		if (values[sz] == 0) {
			channels[names[sz]] = nil;
		} else {
			channels[names[sz]] = values[sz];
		}
	}
}

private void i3_handle_emoteto(mixed *value)
{
	string mud;
	string message;
	object user;

	if (user = TEXT_USERD->find_user(value[5])) {
		user->message(value[6] + "@" + value[2] + " emotes to you: " + value[7]);
	} else {
		mixed *arr;

		arr = ({
			"error",
			5,
			mudname,
			0,
			value[2],
			value[3],
			"unk-user",
			"User not online: " + value[5],
			value
		});

		i3_send_packet(arr);
	}
}

private void i3_handle_error(mixed *value)
{
	LOGD->post_message("system", LOG_ERR, "IntermudD: I3 error: " + mixed_sprint(value));

	if (value[6] == "not-allowed") {
		if (value[7] == "Bad password") {
			mixed *badpkt;

			badpkt = value[8];

			if (badpkt[0] == "startup-req-3") {
				LOGD->post_message("system", LOG_ERR, "IntermudD: Removing rejected password for " + mudname);
				clear_password(mudname);

				rejections++;

				disconnect();

				call_out_unique("i3_connect", 1);
			}
		}
	}
}

private void i3_handle_mudlist(mixed *value)
{
	mapping delta;
	string *names;
	mixed *values;
	int sz;

	delta = value[7];

	names = map_indices(delta);
	values = map_values(delta);
	sz = sizeof(names);

	for ( ; --sz >= 0; ) {
		if (values[sz] == 0) {
			muds[names[sz]] = nil;
		} else {
			muds[names[sz]] = values[sz];
		}
	}
}

private void i3_handle_startup_reply(mixed *value)
{
	string *ch;
	mixed *raw;
	int sz;
	mixed oldpass, newpass;

	LOGD->post_message("system", LOG_NOTICE, "IntermudD: Received startup reply");

	oldpass = query_password(mudname);
	newpass = value[7];

	if (oldpass != newpass) {
		if (oldpass) {
			LOGD->post_message("system", LOG_WARNING, "IntermudD: Password was changed");
		}
	}

	set_password(mudname, newpass);

	call_out_unique("save", 0);

	ch = CHANNELD->query_channels();
	sz = sizeof(ch);

	for (sz = sizeof(ch); --sz >= 0; ) {
		string channel;

		channel = ch[sz];

		if (channels[channel]) {
			if (CHANNELD->query_intermud(channel)) {
				i3_listen_channel(channel);
			} else {
				i3_unlisten_channel(channel);
			}
		}
	}

	pinged = 0;
	ponged = 0;

	call_out_unique("keepalive", 2);
}

private void i3_handle_tell(mixed *value)
{
	mixed username;
	object user;

	username = value[5];

	if (username == 0) {
		if (value[6] == "IntermudD Keepalive" && value[7] == "ping") {
			ponged = 1;
		} else {
			LOGD->post_message("system", LOG_WARNING, "IntermudD: Received spurious 0 tell from I3");
		}
	} else if (user = TEXT_USERD->find_user(value[5])) {
		string msg;

		msg = value[7];

		if (msg == "" || msg[strlen(msg) - 1] != '\n') {
			msg += "\n";
		}

		user->message(value[6] + "@" + value[2] + " tells you: " + msg);
	} else {
		mixed *arr;

		arr = ({
			"error",
			5,
			mudname,
			0,
			value[2],
			value[3],
			"unk-user",
			"User not online: " + value[5],
			value
		});

		i3_send_packet(arr);
	}
}

private void i3_handle_channel_m(mixed *value)
{
	string name;
	string visname;
	string mud;
	string message;
	string header;
	string channel;
	string fullname;
	int cflag;

	message = value[8];

	({ cflag, message }) = sanitize_control(message);

	mud = value[2];
	name = value[3];
	channel = value[6];
	visname = value[7];

	if (to_lower(name) == to_lower(visname)) {
		fullname = visname + "@" + mud;
	} else {
		fullname = visname + " (" + name + ")@" + mud;
	}

	write_secret_log(channel, timestamp() + " " + fullname + ": " + message);

	if (cflag) {
		LOGD->post_message("system", LOG_WARNING, name + "@" + mud + " sent a control character on I3 via " + channel);
		LOGD->post_message("debug", LOG_DEBUG, name + "@" + mud + " sent " + message + " on " + channel);
	}

	if (sscanf(message, "%*s%%^")) {
		message = pinkfish2ansi(message) + "\033[0m";
	}

	if (CHANNELD->test_channel(channel)) {
		CHANNELD->post_message(channel, fullname, message, 1);
	}
}

private void i3_handle_channel_e(mixed *value)
{
	i3_handle_channel_m(value);
}

private void i3_handle_channel_t(mixed *value)
{
	i3_handle_channel_m(value[0 .. 6] + ({ value[12], value[9] }) );
}

private void i3_handle_who_reply(mixed *value)
{
	object user;
	string buffer;
	int i, sz;
	mixed *who_data;

	user = TEXT_USERD->find_user(value[5]);

	if (!user) {
		LOGD->post_message("system", LOG_NOTICE, "I3 sent a who-reply to offline user " + value[5]);
		return;
	}

	who_data = value[6];

	user->message("Users online at " + value[2] + ":\n");

	for (i = 0, sz = sizeof(who_data); i < sz; i++) {
		string visname;
		mixed idle;
		string extra;

		visname = who_data[i][0];
		idle = who_data[i][1];
		extra = who_data[i][2];

		user->message(pinkfish2ansi(visname));

		if (extra) {
			user->message(" - " + pinkfish2ansi(extra));
		}

		if (idle) {
			user->message(" (" + idle + "s)");
		}

		user->message("\n");
	}

	user->message("\033[0m");
}

/* helpers */

private mixed *startup_packet()
{
	int pass;

	mudname = rejections ? base_mudname + (rejections + 1) : base_mudname;

	pass = query_password(mudname);

	if (pass) {
		LOGD->post_message("system", LOG_NOTICE, "Using saved password for " + mudname);
	} else {
		LOGD->post_message("system", LOG_NOTICE, "No saved password for " + mudname);
	}

	return ({
		"startup-req-3",
		5,
		mudname,
		0,
		router,
		0,

		pass,
		0,
		0,

		LOGIN_PORT,
		0,
		0,
		"Kotaka",
		"Kotaka",
		status(ST_VERSION),
		"DGD",
		"(fill in the purpose of the mud here)",
		"(fill in your admin email here)",
		([
			"channel" : 1,
			"tell" : 1
		]),
		([ ])
	});
}

private void process_packet(mixed *value)
{
	if (!is_keepalive_packet(value)) {
		write_secret_log("packet-log", timestamp(millitime()) + " <<< " + mixed_sprint(value));
	}

	switch(value[0]) {
	case "chanlist-reply":
		i3_handle_chanlist_reply(value);
		break;

	case "channel-m":
		i3_handle_channel_m(value);
		break;

	case "channel-e":
		i3_handle_channel_e(value);
		break;

	case "channel-t":
		i3_handle_channel_t(value);
		break;

	case "emoteto":
		i3_handle_emoteto(value);
		break;

	case "error":
		i3_handle_error(value);
		break;

	case "mudlist":
		i3_handle_mudlist(value);
		break;

	case "startup-reply":
		i3_handle_startup_reply(value);
		break;

	case "tell":
		i3_handle_tell(value);
		break;

	case "who-reply":
		i3_handle_who_reply(value);
		break;

	/* we don't care about these right now */
	case "finger-req": /* ignored for privacy */
	case "locate-req": /* ignored for privacy */
	case "ucache-update": /* we don't implement ucache */
	case "who-req": /* ignored for privacy */
	case "chan-who-req": /* ignored for privacy */
		break;

	default:
		i3_bounce_packet(value);
	}
}

private void process()
{
	mixed *arr;
	int len;
	string packet;

	while (strlen(buffer) > 4) {
		len = buffer[3] + (buffer[2] << 8) + (buffer[1] << 16) + (buffer[0] << 24);

		if (strlen(buffer) < len + 4) {
			return;
		}

		buffer = buffer[4 ..];
		packet = buffer[0 .. len - 2];
		buffer = buffer[len ..];

		arr = PARSER_MUDMODE->parse(packet);

		process_packet(arr);
	}
}

private void send_keepalive_packet()
{
	mixed *arr;

	arr = ({
		"tell",
		5,
		mudname,
		0,
		mudname,
		0,
		"IntermudD Keepalive",
		"ping"
	});

	i3_send_packet(arr);
}

/* creator */

static void create()
{
	/* configure this as desired */
	base_mudname = nil;

	rejections = 0;
	router = "*wpr";

	routers = ([ ]);
	muds = ([ ]);
	channels = ([ ]);

	restore();

	legacy_active_checked = 1;

	LOGD->post_message("system", LOG_NOTICE, "IntermudD: Ready to activate.  Please check configuration and then use the \"i3on\" verb to connect");
}

static void destruct()
{
	if (query_conn()) {
		disconnect();
	}
}

/* callouts */

static void keepalive()
{
	call_out_unique("keepalive", 1);

	if (pinged) {
		if (ponged) {
			ponged = 0;

			strikes = 0;

			send_keepalive_packet();
		} else {
			strikes++;

			if (strikes >= 5 && (strikes % 30 == 5)) {
				LOGD->post_message("system", LOG_NOTICE, "IntermudD: Keepalive timeout, strike " + strikes);
			}
		}
	} else {
		LOGD->post_message("system", LOG_NOTICE, "IntermudD: Starting keepalive");

		send_keepalive_packet();

		pinged = 1;
	}
}

static void i3_connect()
{
	string ip;
	int port;

	if (!base_mudname) {
		error("No base mudname configured");
	}

	if (!router) {
		error("No router selected");
	}

	if (!routers[router]) {
		error("No such router: " + router);
	}

	({ ip, port }) = routers[router];

	disconnect();

	connect(ip, port);
}

void reboot()
{
	ACCESS_CHECK(INTERMUD());

	LOGD->post_message("system", LOG_NOTICE, "IntermudD: Reconnecting after reboot");

	if (active) {
		disconnect();
		call_out_unique("i3_connect", 0);
	}
}

/* hooks */

/* objectd hooks */

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	if (!routers) {
		routers = ([ ]);
	}

	clean_passwords();

	if (!legacy_active_checked) {
		active = 1;
		legacy_active_checked = 1;
	}
}

/* userd hooks */

string query_banner(object LIB_CONN conn)
{
	mixed *arr;

	arr = startup_packet();

	conn->set_mode(MODE_RAW);

	return make_packet(arr);
}

int query_timeout(object LIB_CONN conn)
{
	return 3;
}

object select(string input)
{
	return this_object();
}

/* user hooks */

int login(string input)
{
	connection(previous_object());

	buffer = input;

	process();

	return MODE_NOCHANGE;
}

int receive_message(string input)
{
	buffer += input;

	process();

	return MODE_NOCHANGE;
}

int message_done()
{
	return MODE_NOCHANGE;
}

void logout(int quit)
{
	pinged = 0;
	ponged = 0;
	buffer = nil;

	channels = ([ ]);
	muds = ([ ]);

	wipe_callouts("i3_connect");
	wipe_callouts("keepalive");

	if (quit) {
		LOGD->post_message("system", LOG_NOTICE, "IntermudD: Connection closed");
	} else {
		LOGD->post_message("system", LOG_NOTICE, "IntermudD: Connection lost (reconnecting in 5 seconds)");

		call_out_unique("i3_connect", 5);
	}
}

void connect_failed(int refused)
{
	wipe_callouts("i3_connect");
	wipe_callouts("keepalive");

	LOGD->post_message("system", LOG_NOTICE, "IntermudD: Connection failed (reconnecting in 5 seconds)");

	call_out_unique("i3_connect", 5);
}

/* calls */

void activate()
{
	ACCESS_CHECK(INTERMUD() || VERB() || KERNEL() || SYSTEM() || GAME());

	active = 1;

	call_out_unique("i3_connect", 0);
}

void deactivate()
{
	ACCESS_CHECK(INTERMUD() || VERB() || KERNEL() || SYSTEM() || GAME());

	active = 0;

	disconnect();

	wipe_callouts("keepalive");
	wipe_callouts("i3_connect");
}

void save()
{
	string buf;

	ACCESS_CHECK(INTERMUD() || VERB() || KERNEL() || SYSTEM());

	clean_passwords();

	buf = hybrid_sprint( ([
		"router" : router,
		"routers" : routers && map_sizeof(routers) ? routers : nil,
		"passwords" : passwords && map_sizeof(passwords) ? passwords : nil
	]) );

	SECRETD->make_dir(".");
	SECRETD->remove_file("intermud-tmp");
	SECRETD->write_file("intermud-tmp", buf + "\n");
	SECRETD->remove_file("intermud");
	SECRETD->rename_file("intermud-tmp", "intermud");
}

void restore()
{
	mapping map;
	string buf;

	ACCESS_CHECK(INTERMUD() || VERB());

	routers = ([ ]);
	router = nil;

	password = 0;
	passwords = ([ ]);
	rejections = 0;

	buf = SECRETD->read_file("intermud");

	if (buf) {
		catch {
			map = PARSER_VALUE->parse(buf);

			if (map["passwords"]) {
				passwords = map["passwords"];
			}

			if (map["password"]) {
				passwords[base_mudname] = ({ map["password"], time() + 7 * 86400 });
			}

			if (map["routers"]) {
				routers = map["routers"];
			}

			if (map["router"]) {
				router = map["router"];
			}
		} : {
			LOGD->post_message("system", LOG_ERR, "IntermudD: Error parsing Intermud state, resetting");

			SECRETD->remove_file("intermud-bad");
			SECRETD->rename_file("intermud", "intermud-bad");
		}
	}

	clean_passwords();

	if (!routers || !router) {
		reset_routers();
		call_out_unique("save", 0);
	}
}

void send_channel_message(string channel, string sender, string visible, string text)
{
	mixed *arr;

	arr = ({
		"channel-m",
		5,
		mudname,
		sender ? sender : 0,
		0,
		0,
		channel,
		visible,
		text
	});

	i3_send_packet(arr);
}

string *query_channels()
{
	return map_indices(channels);
}

mixed *query_channel(string channel)
{
	return deep_copy(channels[channel]);
}

string *query_muds()
{
	return map_indices(muds);
}

mixed *query_mud(string mud)
{
	return deep_copy(muds[mud]);
}

void send_who(string from, string mud)
{
	mixed *arr;

	ACCESS_CHECK(INTERFACE() || VERB());

	arr = ({
		"who-req",
		5,
		mudname,
		from,
		mud,
		0
	});

	i3_send_packet(arr);
}

void send_tell(string from, string decofrom, string mud, string user, string message)
{
	mixed *arr;

	ACCESS_CHECK(INTERFACE() || VERB());

	arr = ({
		"tell",
		5,
		mudname,
		from,
		mud,
		user,
		decofrom,
		message
	});

	i3_send_packet(arr);
}

void send_emote(string from, string decofrom, string mud, string user, string message)
{
	mixed *arr;

	ACCESS_CHECK(INTERFACE() || VERB());

	arr = ({
		"emoteto",
		5,
		mudname,
		from,
		mud,
		user,
		decofrom + "@" + mudname,
		message
	});

	i3_send_packet(arr);
}

void add_channel(string channel)
{
	mixed *arr;

	ACCESS_CHECK(INTERFACE());

	arr = ({
		"channel-add",
		5,
		mudname,
		0,
		router,
		0,
		channel,
		0
	});

	i3_send_packet(arr);
}

void remove_channel(string channel)
{
	mixed *arr;

	ACCESS_CHECK(INTERFACE());

	arr = ({
		"channel-remove",
		5,
		mudname,
		0,
		router,
		0,
		channel
	});

	i3_send_packet(arr);
}

void listen_channel(string channel)
{
	ACCESS_CHECK(CHANNEL());

	i3_listen_channel(channel);
}

void unlisten_channel(string channel)
{
	ACCESS_CHECK(CHANNEL());

	i3_unlisten_channel(channel);
}

void add_router(string name, string ip, int port)
{
	ACCESS_CHECK(VERB());

	if (!routers) {
		routers = ([ ]);
	}

	routers[name] = ({ ip, port });

	call_out_unique("save", 0);
}

void remove_router(string name)
{
	ACCESS_CHECK(VERB());

	if (!routers) {
		routers = ([ ]);
	}

	routers[name] = nil;

	call_out_unique("save", 0);
}

string *query_routers()
{
	return map_indices(routers);
}

mixed *query_router(string name)
{
	return routers[name];
}

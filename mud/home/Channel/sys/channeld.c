/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2019  Raymond Jennings
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
#include <kotaka/assert.h>
#include <kotaka/checkarg.h>
#include <kotaka/log.h>
#include <kotaka/paths/intermud.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/paths/utility.h>
#include <kotaka/privilege.h>
#include <status.h>

#define SYSTEM_CHANNELS ({ "compile", "debug", "error", "system", "trace" })

inherit "~System/lib/struct/list";
inherit "/lib/string/sprint";
inherit "/lib/copy";
inherit "~System/lib/utility/secretlog";

mapping intermud;	/*< set of channels to be relayed to intermud */
mapping channels;	/*< channel configuration */
mapping subscribers;	/*< channel subscribers */
mapping buffers;

/* buffers: ([ channel name: linked list ]) */

void configure_channels();
void save();
void restore();

static void create()
{
	channels = ([ ]);
	subscribers = ([ ]);
	intermud = ([ ]);

	restore();

	configure_channels();

	save();
}

private void append_node(string channel, string fragment)
{
	mixed **list;

	if (!buffers) {
		buffers = ([ ]);
	}

	list = buffers[channel];

	if (!list) {
		list = ({ nil, nil });
		buffers[channel] = list;
	}

	list_append_string(list, fragment);
}

private void write_node(string channel)
{
	mixed **list;
	mixed *info;

	list = buffers[channel];

	info = SECRETD->file_info("logs/" + channel + ".log");

	if (info && info[0] >= 1 << 30) {
		SECRETD->remove_file("logs/" + channel + ".log.old");
		SECRETD->rename_file("logs/" + channel + ".log", "logs/" + channel + ".log.old");
	}

	SECRETD->make_dir(".");
	SECRETD->make_dir("logs");
	SECRETD->write_file("logs/" + channel + ".log", list_front(list));

	list_pop_front(list);

	if (list_empty(list)) {
		buffers[channel] = nil;
	}
}

static void flush()
{
	while (buffers && map_sizeof(buffers)) {
		string *channels;
		int sz;

		channels = map_indices(buffers);

		sz = sizeof(channels);

		write_node(channels[random(sz)]);

		buffers = nil;
	}
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	call_out_unique("flush", 0);
}

void save()
{
	string buf;

	CONFIGD->make_dir(".");

	buf = hybrid_sprint( ([
		"channels": channels,
		"intermud": intermud
	]) );

	CONFIGD->remove_file("config-tmp");
	CONFIGD->write_file("config-tmp", buf + "\n");
	CONFIGD->remove_file("config");
	CONFIGD->rename_file("config-tmp", "config");
}

void restore()
{
	string buf;
	mapping save;

	buf = CONFIGD->read_file("config");

	if (buf) {
		save = PARSER_VALUE->parse(buf);

		channels = save["channels"];
		intermud = save["intermud"];
	}
}

void add_channel(string channel)
{
	string program;

	CHECKARG(1, channel, "add_channel");

	if (channels[channel]) {
		error("Duplicate channel");
	}

	channels[channel] = ([ ]);

	save();
}

string *query_channels()
{
	return map_indices(channels);
}

void del_channel(string channel)
{
	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	CHECKARG(1, channel, "del_channel");

	if (sizeof(({ channel })
		& SYSTEM_CHANNELS)
	) {
		error("Cannot remove a system channel");
	}

	if (channels[channel]) {
		channels[channel] = nil;
		subscribers[channel] = nil;
	} else {
		error("No such channel");
	}

	save();
}

mixed query_channel_config(string channel, string key)
{
	mapping config;

	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	CHECKARG(channel, 1, "query_channel_config");
	CHECKARG(key, 2, "query_channel_config");

	config = channels[channel];

	if (!config) {
		error("No such channel");
	}

	return deep_copy(config[key]);
}

/* ChannelD only records config data.  If config values have any side
effects, it is the caller's responsibility to enforce them. */
void set_channel_config(string channel, string key, mixed value)
{
	mapping config;

	CHECKARG(channel, 1, "set_channel_config");
	CHECKARG(key, 2, "set_channel_config");

	config = channels[channel];

	if (!config) {
		error("No such channel");
	}

	config[key] = deep_copy(value);

	save();
}

int test_channel(string channel)
{
	CHECKARG(channel, 1, "test_channel");

	return !!channels[channel];
}

void set_intermud(string channel, int true)
{
	if (!intermud) {
		intermud = ([ ]);
	}

	if (true) {
		intermud[channel] = 1;
	} else {
		intermud[channel] = nil;
	}

	INTERMUDD->listen_channel(channel, true);

	save();
}

int query_intermud(string channel)
{
	return !!intermud[channel];
}

void subscribe_channel(string channel, object subscriber)
{
	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	CHECKARG(channel, 1, "subscribe_channel");
	CHECKARG(subscriber, 1, "subscribe_channel");

	if (!channels[channel]) {
		error("No such channel");
	}

	if (!subscribers[channel]) {
		subscribers[channel] = ([ ]);
	}

	subscribers[channel][subscriber] = 1;
}

void unsubscribe_channel(string channel, object subscriber)
{
	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	CHECKARG(channel, 1, "unsubscribe_channel");
	CHECKARG(subscriber, 1, "unsubscribe_channel");

	if (!channels[channel]) {
		error("No such channel");
	}

	if (!subscribers[channel]) {
		return;
	}

	subscribers[channel][subscriber] = nil;

	if (!map_sizeof(subscribers[channel])) {
		subscribers[channel] = nil;
	}
}

int is_subscribed(string channel, object user)
{
	mixed *data;
	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	CHECKARG(channel, 1, "is_subscribed");
	CHECKARG(user, 1, "is_subscribed");

	if (!channels[channel]) {
		error("No such channel");
	}

	if (!subscribers[channel]) {
		return 0;
	}

	return !!subscribers[channel][user];
}

string *query_subscriptions(object subscriber)
{
	string *names;
	mixed *values;
	int i;

	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	CHECKARG(subscriber, 1, "query_subscriptions");

	names = map_indices(subscribers);
	values = map_values(subscribers);

	for (i = 0; i < sizeof(names); i++) {
		if (!values[i][subscriber]) {
			names[i] = nil;
		}
	}

	return names - ({ nil });
}

object *query_subscribers(string channel)
{
	if (!channels[channel]) {
		error("No such channel");
	}

	if (subscribers[channel]) {
		return map_indices(subscribers[channel]);
	} else {
		return ({ });
	}
}

/**********************/
/* message management */
/**********************/

private string *timestamps(mixed *mtime)
{
	string ctime;

	string date;
	string hms;
	mixed msec;

	mtime = millitime();

	ctime = ctime(mtime[0]);

	/* 012345678901234567890123456789 */
	/* Tue Aug  3 14:40:18 1993 */

	date = ctime[0 .. 9] + " " + ctime[20 .. 23];
	hms = ctime[11 .. 18];
	msec = mtime[1];
	msec *= 1000.0;
	msec = floor(msec + 0.5);
	msec = "000" + msec;
	msec = msec[strlen(msec) - 3 ..];

	return ({ date + hms + "." + msec, hms });
}

void post_message(string channel, string sender, string message, varargs int norelay)
{
	object *send_list;
	mixed *mtime;
	string *timestamps;

	ASSERT(message);

	if (!channels[channel]) {
		error("No such channel");
	}

	mtime = millitime();

	if (!norelay) {
		if (intermud && intermud[channel]) {
			object user;

			user = TEXT_USERD->find_user(sender);

			if (user) {
				INTERMUDD->send_channel_message(channel, sender, user->query_titled_name(), message);
			} else {
				INTERMUDD->send_channel_message(channel, sender, sender, message);
			}

			return;
		}
	}

	timestamps = timestamps(mtime);

	write_secret_log(channel, timestamps[0] + " " + message);

	if (subscribers[channel]) {
		send_list = map_indices(subscribers[channel]);
	} else {
		return;
	}

	if (send_list) {
		int sz;

		for (sz = sizeof(send_list); --sz >= 0; ) {
			send_list[sz]->channel_message(channel, mtime, sender, message);
		}
	}
}

void configure_channels()
{
	string *channels;
	int sz;

	ACCESS_CHECK(INTERFACE() || CHANNEL());

	/* error: errord, used for runtime errors */
	/* trace: errord, used for stack traces */
	/* compile: errord, used for compile errors */

	channels = SYSTEM_CHANNELS;

	for (sz = sizeof(channels); --sz >= 0; ) {
		if (!test_channel(channels[sz])) {
			add_channel(channels[sz]);
		}
	}
}

/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/checkarg.h>
#include <kotaka/log.h>
#include <kotaka/paths/kotaka.h>
#include <kotaka/paths/intermud.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

mapping intermud;	/*< set of channels to be relayed to intermud */
mapping channels;	/*< channel configuration */
mapping subscribers;	/*< channel subscribers */
mapping locks;		/*< configuration locks */

/*
Config keys:

Used by ChannelD

int channel_color
	ANSI color for channel name
int prefix_color
	ANSI color for prefix
int text_color
	ANSI color for text

Standard with significance

string join_access
	guest, player, wizard, admin, system
string post_access
	guest, player, wizard, admin, system
mapping bans
	username : 1
*/
void configure_channels();

static void create()
{
	channels = ([ ]);
	subscribers = ([ ]);
	locks = ([ ]);
	intermud = ([ ]);

	configure_channels();
}

/**********************/
/* channel management */
/**********************/

void add_channel(string channel, varargs int lock)
{
	string program;

	program == previous_program();

	CHECKARG(1, channel, "add_channel");

	if (channels[channel]) {
		error("Duplicate channel");
	}

	channels[channel] = ([ ]);

	if (lock) {
		locks[channel] = program;
	}
}

string *query_channels()
{
	return map_indices(channels);
}

void del_channel(string channel)
{
	ACCESS_CHECK(PRIVILEGED() || INTERFACE());

	CHECKARG(1, channel, "del_channel");

	if (locks[channel]) {
		ACCESS_CHECK(locks[channel] == previous_program());
	}

	if (channels[channel]) {
		channels[channel] = nil;
		subscribers[channel] = nil;
	} else {
		error("No such channel");
	}
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

	return SUBD->deep_copy(config[key]);
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

	config[key] = SUBD->deep_copy(value);
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
}

int query_intermud(string channel)
{
	return !!intermud[channel];
}

/*******************/
/* user management */
/*******************/

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

string setcolor(int color)
{
	if (color == -1) {
		return "\033[0m";
	} else if (color < 8) {
		return "\033[22;3" + color + "m";
	} else if (color < 16) {
		return "\033[1;3" + (color - 8) + "m";
	}
}

void post_message(string channel, string sender, string message, varargs int norelay)
{
	object *send_list;
	int time;
	string stamp;

	if (!norelay) {
		if (intermud && intermud[channel]) {
			INTERMUDD->send_channel_message(channel, sender, message);

			return;
		}
	}

	time = time();

	write_file("log/log-" + (time - time % 86400) + "-" + channel, time + ": " + channel + ": " + (sender ? sender + ": " : "") + message + "\n");

	stamp = ctime(time)[11 .. 18];

	if (subscribers[channel]) {
		send_list = map_indices(subscribers[channel]);
	} else {
		return;
	}

	if (send_list) {
		int sz;
		int index;

		sz = sizeof(send_list);

		for (index = 0; index < sz; index++) {
			send_list[index]->channel_message(channel, stamp, sender, message);
		}
	}
}

int is_locked(string channel)
{
	return !!locks[channel];
}

void configure_channels()
{
	string *channels;
	int i, sz;

	ACCESS_CHECK(INTERFACE() || KOTAKA());

	/* error: errord, used for runtime errors */
	/* trace: errord, used for stack traces */
	/* compile: errord, used for compile errors */

	channels = ({ "error", "trace", "compile", "system" });

	sz = sizeof(channels);

	for (i = 0; i < sz; i++) {
		if (!test_channel(channels[i])) {
			add_channel(channels[i], 1);
		}
	}

	channels = query_channels() - channels;

	sz = sizeof(channels);

	for (i = 0; i < sz; i++) {
		del_channel(channels[i]);
	}
}

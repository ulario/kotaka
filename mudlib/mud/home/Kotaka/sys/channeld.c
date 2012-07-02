#include <kernel/kernel.h>

#include <kotaka/privilege.h>
#include <kotaka/checkarg.h>
#include <kotaka/paths.h>
#include <kotaka/channel.h>

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
	username to expiry (-1 is permanent)
*/

/**********************/
/* channel management */
/**********************/

/** creates a new channel */
void add_channel(string channel, varargs int lock)
{
	string program;
	
	program == previous_program();
	
	ACCESS_CHECK(PRIVILEGED());

	CHECKARG(1, channel, "add_channel");

	if (channels[channel]) {
		error("Duplicate channel");
	}

	channels[channel] = ([ ]);

	if (lock) {
		locks[channel] = program;
	}
}

static void create()
{
	channels = ([ ]);
	subscribers = ([ ]);
	locks = ([ ]);

	add_channel("error", 1);
	add_channel("debug", 1);
	add_channel("trace", 1);
	add_channel("warning", 1);
	add_channel("compile", 1);
	add_channel("connection", 1);
}

/** lists all the channels */
string *query_channels()
{
	ACCESS_CHECK(PRIVILEGED());

	return map_indices(channels);
}

/** deletes a channel */
void del_channel(string channel)
{
	ACCESS_CHECK(PRIVILEGED());

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

/** queries a channel config value */
mixed query_channel_config(string channel, string key)
{
	mapping config;

	ACCESS_CHECK(PRIVILEGED());

	CHECKARG(channel, 1, "query_channel_config");
	CHECKARG(key, 2, "query_channel_config");

	config = channels[channel];

	if (!config) {
		error("No such channel");
	}

	return SUBD->deep_copy(config[key]);
}

/** sets a channel config entry

\note

ChannelD only records config data.  If config values have any side
effects, it is the caller's responsibility to enforce them.

*/
void set_channel_config(string channel, string key, mixed value)
{
	mapping config;

	ACCESS_CHECK(PRIVILEGED());

	CHECKARG(channel, 1, "set_channel_config");
	CHECKARG(key, 2, "set_channel_config");

	config = channels[channel];

	if (!config) {
		error("No such channel");
	}

	config[key] = SUBD->deep_copy(value);
}

/** returns true if the channel exists */
int test_channel(string channel)
{
	ACCESS_CHECK(PRIVILEGED());
	CHECKARG(channel, 1, "test_channel");

	return !!channels[channel];
}

/*******************/
/* user management */
/*******************/

/** subscribes to a channel

Access control is the caller's responsiblity.

*/
void subscribe_channel(string channel, object subscriber)
{
	ACCESS_CHECK(PRIVILEGED());

	CHECKARG(channel, 1, "subscribed_user");
	CHECKARG(subscriber, 1, "subscribe_user");

	if (!channels[channel]) {
		error("No such channel");
	}

	if (!subscribers[channel]) {
		subscribers[channel] = ([ ]);
	}

	subscribers[channel][subscriber] = 1;
}

/** unsubscribe a user from a channel */
void unsubscribe_user(string channel, object subscriber)
{
	ACCESS_CHECK(PRIVILEGED());

	CHECKARG(channel, 1, "unsubscribe_user");
	CHECKARG(subscriber, 1, "unsubscribe_user");

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

/** checks if a user is subscribed */
int is_subscribed(string channel, object user)
{
	mixed *data;
	ACCESS_CHECK(PRIVILEGED());

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

/** lists all channels an object is subscribed to */
string *query_subscriptions(object subscriber)
{
	string *names;
	mixed *values;
	int i;
	
	ACCESS_CHECK(PRIVILEGED());

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

/* sends a message to a channel, optionally from a user */
void post_message(string channel, string sender, string message)
{
	object *send_list;

	send_list = subscribers[channel];

	if (send_list) {
		int sz;
		int index;

		sz = sizeof(send_list);

		for (index = 0; index < sz; index++) {
			send_list[index]->channel_message(channel, sender, message);
		}
	}
}

int is_locked(string channel)
{
	return !!locks[channel];
}

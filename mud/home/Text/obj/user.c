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
#include <kotaka/assert.h>
#include <kotaka/paths/ansi.h>
#include <kotaka/paths/account.h>
#include <kotaka/paths/channel.h>
#include <kotaka/paths/utility.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <kotaka/ustate.h>

inherit "~/lib/user";
inherit "~/lib/log";
inherit "/lib/string/replace";

object mobile;
object body;
int destructing;
int quitting;
int logging_out;
string username;

static void unsubscribe_channels();

static void create(int clone)
{
	if (clone) {
		::create();
	}
}

void send_out(string msg)
{
	ASSERT(msg);

	::send_out(msg);

	if (!query_top_state()->forbid_log_outbound()) {
		log_message(">>> " + msg);
	}
}

void dispatch_wiztool(string line)
{
	ACCESS_CHECK(TEXT());

	PROXYD->get_wiztool(query_name())->input(line);
}

static void destruct(int clone)
{
	if (clone) {
		destructing = 1;

		if (!logging_out) {
			disconnect();
		}
	}
}

string query_username()
{
	return username;
}

int query_class()
{
	return TEXT_SUBD->query_user_class(username);
}

string query_titled_name()
{
	return TEXT_SUBD->query_titled_name(username);
}

string query_name()
{
	return username;
}

void set_mobile(object new_mobile)
{
	ACCESS_CHECK(TEXT() || GAME() || VERB());

	mobile = new_mobile;
}

object query_mobile()
{
	return mobile;
}

void set_body(object new_body)
{
	object *mobiles;

	ACCESS_CHECK(TEXT() || GAME() || VERB());

	if (body) {
		mobiles = body->query_property("mobiles");

		if (mobiles) {
			mobiles -= ({ this_object(), nil });

			if (!sizeof(mobiles)) {
				mobiles = nil;
			}

			body->set_property("mobiles", mobiles);
		}
	}

	body = new_body;

	if (body) {
		mobiles = body->query_property("mobiles");

		if (!mobiles) {
			mobiles = ({ });
		}

		mobiles -= ({ nil });
		mobiles |= ({ this_object() });
		body->set_property("mobiles", mobiles);
	}
}

object query_body()
{
	return body;
}

object query_base_conn()
{
	object conn;

	conn = query_conn();

	while (conn && conn <- LIB_USER) {
		conn = conn->query_conn();
	}

	return conn;
}

void quit(string cause)
{
	unsubscribe_channels();

	quitting = 1;

	switch(cause) {
	case "quit": /* user logged out */
		message("Come back soon.\n");
		break;

	case "banned": /* user was banned */
		/* user's message handled by ban command */
		break;

	case "sitebanned": /* user was sitebanned */
		{
			mapping ban;
			string message;
			mixed expire;

			ban = BAND->check_siteban(query_ip_number(query_base_conn()));

			message("You have just been sitebanned.\n");

			if (message = ban["message"]) {
				message(message + "\n");
			}

			expire = ban["expire"];

			if (expire != nil && expire != -1) {
				int remaining;

				remaining = expire - time();

				if (remaining < 60) {
					message("(expires in " + remaining + " seconds)\n");
				} else if (remaining < 3600) {
					message("(expires in " + ((remaining + 59) / 60) + " minutes)\n");
				} else if (remaining < 86400) {
					message("(expires in " + ((remaining + 3599) / 3600) + " hours)\n");
				} else {
					message("(expires in " + ((remaining + 86399) / 86400) + " days)\n");
				}
			}
		}
		break;

	case "bumped": /* user logged in on another connection */
		break; /* handled by new user's login */

	case "badpass": /* password authentication failed */
		break; /* handled by login */

	case "kicked": /* user was kicked */
		break; /* handled by kick */

	case "nuked": /* user's account was nuked */
		break; /* handled by nuke */

	case "timeout": /* user timed out at login process */
		break; /* handled by login */

	case "idle": /* user idled out */
		break;
	}

	disconnect();

	quitting = 0;
}

void logout(int dest)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	logging_out = 1;

	if (username && !quitting) {
		if (dest) {
			/* connection object was destructed, manual logout */
		} else {
			/* remote closure, we're linkdead */
		}
	}

	unsubscribe_channels(); /* we won't be able to receive channel messages once we nuke our ustate tree */
	::logout();

	if (!destructing) {
		destruct_object(this_object());
	}
}

private void do_banner()
{
	string *files;
	string ansi;
	int sz;
	int level;
	int rnd;
	int splash;

	files = get_dir("~/data/splash/telnet_banners/chars/*")[0];
	sz = sizeof(files);
	splash = random(sz);

	ansi = read_file("~/data/splash/telnet_banners/ansi/" + files[splash]);

	if (!ansi) {
		ansi = ANSI_SUBD->simple_ansify(
			read_file("~/data/splash/telnet_banners/chars/" + files[splash]),
			read_file("~/data/splash/telnet_banners/fgcolor/" + files[splash]),
			read_file("~/data/splash/telnet_banners/bgcolor/" + files[splash])
		);

		write_file("~/data/splash/telnet_banners/ansi/" + files[splash], ansi);
	};

	splash++;

	ansi = replace(ansi, "\n", "\r\n");

	ASSERT(ansi);

	::send_out(ansi);
}

private void do_login()
{
	connection(previous_object());

	do_banner();

	set_mode(MODE_ECHO);

	set_root_state(clone_object(USTATE_DIR + "/login"));

	TEXT_USERD->add_guest(this_object());
}

private int do_receive(string msg)
{
	int ret;
	string logmsg;
	mixed *mtime;

	logmsg = msg;

	if (!query_top_state()->forbid_log_inbound()) {
		log_message("<<< " + msg);
	}

	ret = ::receive_message(msg);

	set_mode(ret);

	return ret;
}

int login(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	do_login();

	if (str != nil) {
		return do_receive(str);
	}

	return MODE_NOCHANGE;
}

int receive_message(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	return do_receive(str);
}

/* ctime format: */
/* Tue Aug  3 14:40:18 1993 */
/* 012345678901234567890123 */

void channel_message(string channel, mixed *mtime, string sender, string message)
{
	string stamp;

	ACCESS_CHECK(previous_program() == CHANNELD);

	stamp = ctime(mtime[0])[11 .. 15];

	if (sender) {
		if (message) {
			send_out("[\033[1m" + channel + "\033[0m] \033[1;32m" + stamp +
				"\033[0m \033[1;34m" + sender + "\033[0m: " + message + "\n");
		} else {
			send_out("[\033[1m" + channel + "\033[0m] \033[1;32m" + stamp +
				"\033[0m \033[1;34m" + sender + "\033[0m\n");
		}
	} else {
		send_out("[\033[1m" + channel + "\033[0m] \033[1;32m" + stamp +
			"\033[0m " + message + "\n");
	}
}

static void subscribe_channels()
{
	string *channels;

	channels = ACCOUNTD->query_account_property(username, "channels");

	if (channels) {
		int sz;

		for (sz = sizeof(channels); --sz >= 0; ) {
			if (CHANNELD->test_channel(channels[sz])) {
				CHANNELD->subscribe_channel(channels[sz], this_object());
			} else {
				message("Warning: " + channels[sz] + " does not exist.\n");
			}
		}
	}
}

static void unsubscribe_channels()
{
	string *subscriptions;
	int sz;
	object this;

	this = this_object();

	subscriptions = CHANNELD->query_subscriptions(this);

	for (sz = sizeof(subscriptions); --sz >= 0; ) {
		CHANNELD->unsubscribe_channel(subscriptions[sz], this);
	}
}

object query_telnet_obj()
{
	object conn;

	conn = query_conn();

	while (conn && conn <- LIB_USER) {
		if (conn <- "~Text/obj/filter/telnet") {
			return conn;
		}
		conn = conn->query_conn();
	}
}

object query_mudclient_obj()
{
	object conn;

	conn = query_conn();

	while (conn && conn <- LIB_USER) {
		if (conn <- "~Text/obj/filter/mudclient") {
			return conn;
		}
		conn = conn->query_conn();
	}
}

void login_user(string name)
{
	object this;

	ACCESS_CHECK(TEXT());

	this = this_object();

	username = name;

	if (TEXT_USERD->query_is_guest(this)) {
		TEXT_USERD->promote_guest(username, this);
	} else {
		TEXT_USERD->add_user(username, this);
	}

	set_mode(MODE_ECHO);
	subscribe_channels();
}

void logout_user()
{
	ASSERT(username);

	TEXT_USERD->demote_user(username);

	username = nil;
}

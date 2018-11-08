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
inherit "~/lib/logging";
inherit "/lib/string/replace";

object mobile;
object body;
int keepalive;
int quitting;

string username;

static void create(int clone)
{
	if (clone) {
		::create();
	}
}

void send_out(string msg)
{
	string user;

	ASSERT(msg);

	::send_out(msg);

	if (!sscanf(msg, "%*s\n")) {
		msg += "\n";
	}

	user = whoami();

	if (!user) {
		user = "(anonymous)";
	}

	if (!query_top_state()->forbid_log_outbound()) {
		log_message_out(user, msg);
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
		::destruct();
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

void set_username(string new_username)
{
	ACCESS_CHECK(TEXT());

	username = new_username;
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
			string msg;

			msg = BAND->check_siteban_message(query_base_conn());

			if (msg) {
				message(msg + "\n");
			}
		}
		break;
	case "bumped": /* user logged in on another connection */
		/* handled by new user's login */
		break;
	case "badpass": /* password authentication failed */
		break;
	case "kicked": /* user was kicked */
		break; /* kick will handle this */
	case "nuked": /* user's account was nuked */
		break;
	case "timeout": /* user timed out at login process */
		break; /* handled by start ustate */
	case "idle": /* user idled out */
		break;
	}

	disconnect();

	if (this_object()) {
		destruct_object(this_object());
	}
}

void logout(int dest)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	if (!username || quitting) {
	} else {
		if (dest) {
			/* connection object was destructed, manual logout */
			message("Come back soon.\n");
			TEXT_SUBD->send_logout_message(username, " gets disconnected.");
		} else {
			/* remote closure, we're linkdead */
			TEXT_SUBD->send_logout_message(username, " goes linkdead.");
		}
	}

	::logout();

	if (this_object()) {
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

	set_root_state(clone_object(USTATE_DIR + "/start"));

	TEXT_USERD->add_guest(this_object());
}

private int do_receive(string msg)
{
	int ret;
	string user;
	string logmsg;
	mixed *mtime;

	user = whoami();

	logmsg = msg;

	if (!sscanf(logmsg, "%*s\n")) {
		logmsg += "\n";
	}

	if (!query_top_state()->forbid_log_inbound()) {
		log_message_in(user, logmsg);
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

void channel_message(string channel, string stamp, string sender, string message)
{
	ACCESS_CHECK(previous_program() == CHANNELD);

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

void subscribe_channels()
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

/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kotaka/privilege.h>
#include <kernel/user.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/assert.h>
#include <kernel/access.h>

#include <text/paths.h>

inherit "~/lib/user";

object mobile;
object body;
int keepalive;

string username;

mapping aliases;
string *disabled;

static void create(int clone)
{
	if (clone) {
		::create();

		aliases = ([ ]);
		disabled = ({ });
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
	}
}

string query_username()
{
	return username;
}

int query_class()
{
	return "~/sys/subd"->query_user_class(username);
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

void set_uid(int new_uid)
{
	ACCESS_CHECK(TEXT());

	LOGD->post_message("game", LOG_INFO, "User has new UID: " + new_uid);
}

void set_mobile(object new_mobile)
{
	ACCESS_CHECK(TEXT() || GAME());

	mobile = new_mobile;
}

object query_mobile()
{
	return mobile;
}

void set_body(object new_body)
{
	ACCESS_CHECK(TEXT() || GAME());

	body = new_body;
}

object query_body()
{
	return body;
}

void logout(int quit)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	::logout(quit);
}

int login(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);
	ASSERT(str == nil);

	connection(previous_object());

	set_mode(MODE_ECHO);

	set_root_state(clone_object("~/obj/ustate/start"));

	return MODE_NOCHANGE;
}

int receive_message(string str)
{
	int ret;
	object conn, conn2;
	string ip, user;

	ACCESS_CHECK(previous_program() == LIB_CONN);
	conn = previous_object();

	while (conn <- LIB_USER) {
		conn = conn->query_conn();
	}

	ip = query_ip_number(conn);

	if (!ip)
		ip = "nil";

	user = username ? username : ip;

	SECRETD->write_file("log/log-" + user, "[" + SUBD->pmtime(millitime()) + "] " + user + ": " + str + "\n");

	ret = ::receive_message(str);

	if (ret == MODE_DISCONNECT) {
		INITD->message("User is dying...");
	}

	set_mode(ret);

	return ret;
}

private string query_brief(object obj)
{
	string out;

	out = obj->query_property("bdesc");

	if (out) {
		return out;
	} else {
		return "an undescript item";
	}
}

void set_aliases(mapping new_aliases)
{
	ACCESS_CHECK(previous_object() == query_root());

	aliases = new_aliases;
}

void set_disabled_aliases(string *new_disabled)
{
	ACCESS_CHECK(previous_object() == query_root());

	disabled = new_disabled;
}

mapping query_aliases()
{
	return aliases[..];
}

string *query_disabled_aliases()
{
	return disabled[..];
}

void add_alias(string prefix, string output)
{
	ACCESS_CHECK(TEXT());

	PERMISSION_CHECK(prefix != "aliases");
	PERMISSION_CHECK(prefix != "alias");
	PERMISSION_CHECK(prefix != "unalias");

	aliases[prefix] = output;
}

void del_alias(string prefix)
{
	ACCESS_CHECK(TEXT());

	aliases[prefix] = nil;
}

string query_alias(string prefix)
{
	return aliases[prefix];
}

void quit()
{
	disconnect();
}

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

void channel_message(string channel, string sender, string message)
{
	mixed ccolor;
	mixed tcolor;

	ACCESS_CHECK(previous_program() == CHANNELD);

	ccolor = CHANNELD->query_channel_config(channel, "channel_color");
	tcolor = CHANNELD->query_channel_config(channel, "text_color");

	if (ccolor == nil) {
		ccolor = -1;
	}
	if (tcolor == nil) {
		tcolor = -1;
	}

	ccolor = CHANNELD->setcolor(ccolor);
	tcolor = CHANNELD->setcolor(tcolor);

	send_out("[" + ccolor + channel + "\033[0m] "
		+ sender + tcolor + ": " + message + "\n");
}

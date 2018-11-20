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
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;

inherit conn LIB_CONN;
inherit user LIB_USER;

void disconnect();

static void create()
{
	user::create();
	conn::create("telnet");
}

static void self_destruct()
{
	destruct_object(this_object());
}

static void self_disconnect()
{
	disconnect();
}

int login(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN
		|| calling_object() == this_object());

	connection(previous_object());

	return conn::receive_message(nil, str);
}

/*

/kernel/lib/connection

static void close(mixed *tls, int dest)
{
    mixed stack;
    mixed ticks;

    stack = status(ST_STACKDEPTH);
    ticks = status(ST_TICKS);

    rlimits (-1; -1) {
	if (user) {
	    catch {
		rlimits (stack; ticks) {
		    user->logout(dest);
		}
	    }
	}
	if (!dest) {
	    destruct_object(this_object());
	}
    }
}

/kernel/obj/user

void logout(int quit)
{
    if (previous_program() == LIB_CONN && --nconn == 0) {
	if (query_conn()) {
	    if (quit) {
		tell_audience(Name + " logs out.\n");
	    } else {
		tell_audience(Name + " disconnected.\n");
	    }
	}
	::logout();
	if (wiztool) {
	    destruct_object(wiztool);
	}
	destruct_object(this_object());
    }
}

*/

void logout(int quit)
{
	ACCESS_CHECK(previous_program() == LIB_CONN
		|| calling_object() == this_object());

	/* LIB_CONN will self destruct if quit is false */
	close(nil, quit);

	if (quit) {
		destruct_object(this_object());
	}
}

int receive_message(string str)
{
	int newmode;
	string err;

	ACCESS_CHECK(previous_program() == LIB_CONN
		|| calling_object() == this_object());

	return conn::receive_message(nil, str);
}

int message_done()
{
	ACCESS_CHECK(previous_program() == LIB_CONN
		|| calling_object() == this_object());

	conn::message_done(nil);

	return MODE_NOCHANGE;
}

void receive_datagram(string packet)
{
	ACCESS_CHECK(previous_program() == LIB_CONN
		|| calling_object() == this_object());

	conn::receive_datagram(nil, packet);
}

int message(string str)
{
	ACCESS_CHECK(previous_object() == query_user()
		|| calling_object() == this_object());

	return user::message(str);
}

void disconnect()
{
	ACCESS_CHECK(previous_object() == query_user()
		|| calling_object() == this_object());

	user::disconnect();
}

void datagram_challenge(string str)
{
	ACCESS_CHECK(previous_object() == query_user()
		|| calling_object() == this_object());

	user::datagram_challenge(str);
}

int datagram(string str)
{
	ACCESS_CHECK(previous_object() == query_user()
		|| calling_object() == this_object());

	return user::datagram(str);
}

static void timeout()
{
	::timeout(allocate(DRIVER->query_tls_size()));
}

void set_mode(int new_mode)
{
	object conn;

	ACCESS_CHECK(previous_program() == LIB_CONN || SYSTEM() ||
		calling_object() == this_object());

	if (!this_object() || !query_conn() || new_mode == MODE_NOCHANGE) {
		return;
	}

	/* we have to do it this way because set_mode is also called to handle return values for hooks in the user object */
	/* this can happen more than once in a connection chain, so we need to intercept disconnects and do them */
	/* in a 0 callout to avoid double destruction */

	/* since network events are asynchronous anyway we aren't causing any actual harm */
	if (new_mode == MODE_DISCONNECT) {
		query_conn()->set_mode(MODE_BLOCK);
		conn::close(nil, 1);
		call_out("self_disconnect", 0);
		return;
	}

	query_conn()->set_mode(new_mode);
}

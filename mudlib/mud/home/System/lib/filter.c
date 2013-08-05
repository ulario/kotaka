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
#include <kernel/user.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;

inherit conn LIB_CONN;
inherit user LIB_USER;

#define DESTRUCT_OPEN		1
#define DESTRUCT_CLOSE		2
#define DESTRUCT_SET_MODE	3
#define DESTRUCT_DISCONNECT	4
#define DESTRUCT_REBOOT		5

static void create()
{
	user::create();
	conn::create("telnet");
}

/*********************************/
/* user hooks (conn::user->func) */
/*********************************/

/*

LIB_USER destructs on:

*/

/*
login
logout
receive_message
message_done
open_datagram
receive_datagram
*/

int login(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN
		|| calling_object() == this_object());

	connection(previous_object());
	open(nil);

	/* LIB_CONN does an implicit call to set_mode */
	return conn::receive_message(nil, str);
}

void logout(int quit)
{
	ACCESS_CHECK(previous_program() == LIB_CONN
		|| calling_object() == this_object());

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

	/* LIB_CONN does an implicit call to set_mode */
	return conn::receive_message(nil, str);
}

int message_done()
{
	ACCESS_CHECK(previous_program() == LIB_CONN
		|| calling_object() == this_object());

	conn::message_done(nil);

	return MODE_NOCHANGE;
}

# ifndef SYS_NETWORKING
void open_datagram()
{
	ACCESS_CHECK(previous_program() == LIB_CONN
		|| calling_object() == this_object());

	conn::open_datagram(nil);
}
# endif

void receive_datagram(string packet)
{
	ACCESS_CHECK(previous_program() == LIB_CONN
		|| calling_object() == this_object());

	conn::receive_datagram(nil, packet);
}

/***************************************/
/* connection hooks (user::conn->func) */
/***************************************/

/*

LIB_CONN self destructs on:

set_mode
	on MODE_DISCONNECT

open
	if userd returns -1 for timeout

close
	if not !dest

disconnect
	unconditional

reboot
	unconditional

*/

/*
message
disconnect
datagram_challenge
datagram
timeout
*/

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

/*************************************/
/* connection api (this->conn::func) */
/*************************************/

void set_mode(int new_mode)
{
	object conn;

	ACCESS_CHECK(previous_program() == LIB_CONN || SYSTEM() ||
		calling_object() == this_object());

	if (!this_object() || !query_conn() || new_mode == MODE_NOCHANGE) {
		return;
	}

	if (new_mode == MODE_DISCONNECT) {
		query_conn()->set_mode(MODE_BLOCK);
		conn::close(nil, 1);
		call_out("self_disconnect", 0);
		return;
	}

	query_conn()->set_mode(new_mode);
}

static void self_destruct()
{
	destruct_object(this_object());
}

static void self_disconnect()
{
	disconnect();
}

/******************************/
/* user api (this->user::func) */
/******************************/

/***********/
/* Unknown */
/***********/

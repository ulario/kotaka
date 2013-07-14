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
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <text/paths.h>

inherit TEXT_LIB_USTATE;

#define STATE_GETCHOICE		1
#define STATE_REGISTER_GETNAME	2
#define STATE_REGISTER_GETPASS	3
#define STATE_REGISTER_CHKPASS	4
#define STATE_LOGIN_GETNAME	5
#define STATE_LOGIN_CHKPASS	6

string name;
string password;
int state;
int stopped;
int reading;
int dead;

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
}

private void prompt()
{
	send_out("Welcome to Ulario.\n");
	send_out("Please choose one of the following options:\n");
	send_out("1. login\n");
	send_out("2. register a new account\n");
	send_out("3. connect as a guest\n");
	send_out("4. get help for this menu\n");
	send_out("5. disconnect\n> ");
}

static void timeout()
{
	send_out("\n\nSorry, but you took too long.\n");
	query_user()->quit();
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());
	call_out("timeout", 120);
	state = STATE_GETCHOICE;
}

void stop()
{
	ACCESS_CHECK(previous_object() == query_user());

	stopped = 1;
}

void go()
{
	ACCESS_CHECK(previous_object() == query_user());

	stopped = 0;

	if (!reading) {
		prompt();
	}
}

void pre_end()
{
	ACCESS_CHECK(previous_object() == query_user());

	dead = 1;
}

void end()
{
	ACCESS_CHECK(previous_object() == query_user());

	destruct_object(this_object());
}

private void do_help()
{
	send_out(read_file("~/data/doc/guest_help"));
}

void receive_in(string input)
{
	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	switch(state) {
	case STATE_GETCHOICE:
		switch(input) {
		case "1": /* login */
			push_state(clone_object("login"));
			break;

		case "2": /* register */
			push_state(clone_object("register"));
			break;

		case "3": /* guest */
			swap_state(clone_object("shell"));
			return;

		case "4":
			do_help();
			break;

		case "5":
			send_out("Thanks for visiting.\n");
			query_user()->quit();
			return;

		default:
			send_out("That is not a valid option.\n");
		}

		break;
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}

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
#include <kotaka/paths/account.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/text.h>
#include <kotaka/assert.h>
#include <kotaka/privilege.h>

inherit TEXT_LIB_USTATE;

#define STATE_GETPASS	1
#define STATE_CHKPASS	2

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
	switch(state) {
	case STATE_GETPASS:
		send_out("Please choose a password: ");
		break;

	case STATE_CHKPASS:
		send_out("Please confirm your password: ");
		break;
	}
}

void set_name(string new_name)
{
	ASSERT(!name);

	name = new_name;
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	state = STATE_GETPASS;

	query_user()->set_mode(MODE_NOECHO);
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

private int authorized()
{
	user = query_user();

	if (name != user->query_name()) {
		switch(TEXT_SUBD->query_user_class(name)) {
		case 4: /* owner */
			if (user->query_username() != "admin") {
				return 0;
			}
			break;

		case 3: /* administrator */
			if (user->query_username() != "admin") {
				return 0;
			}
			break;

		default:
			if (user->query_class() < 3) {
				return 0;
			}
			break;
		}
	}

	return 1;
}

void receive_in(string input)
{
	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	switch(state) {
	case STATE_GETPASS:
		send_out("\n");

		if (!ACCOUNTD->query_is_registered(name)) {
			send_out("Whoops, the account you're changing the password on just poofed.\n");
			query_user()->set_mode(MODE_ECHO);
			pop_state();
			return;
		}

		if (!authorized()) {
			send_out("You are no longer authorized to change that password.\n");
			pop_state();
			return;
		}

		password = input;
		state = STATE_CHKPASS;
		break;

	case STATE_CHKPASS:
		send_out("\n");
		query_user()->set_mode(MODE_ECHO);

		if (!ACCOUNTD->query_is_registered(name)) {
			send_out("Whoops, the account you're changing the password on just poofed.\n");
			pop_state();
			return;
		}
		if (!authorized()) {
			send_out("You are no longer authorized to change that password.\n");
			pop_state();
			return;
		}

		if (input != password) {
			send_out("Password mismatch.\n");
			pop_state();
			return;
		} else {
			ACCOUNTD->change_password(name, password);
			pop_state();
			return;
		}
		break;
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}

void end()
{
	destruct_object(this_object());
}

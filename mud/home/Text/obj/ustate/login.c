/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/privilege.h>

inherit TEXT_LIB_USTATE;

#define STATE_GETNAME	1
#define STATE_GETPASS	2
#define STATE_CHKPASS	3
#define STATE_CHKDUPE	4

string name;
string password;
int silent;

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
	case STATE_GETNAME:
		send_out("Login: ");
		break;

	case STATE_CHKPASS:
		send_out("Password: ");
		break;
	}
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	if (query_user()->query_username()) {
		send_out("You are already logged in.\n");
		pop_state();
		return;
	}

	state = STATE_GETNAME;
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
	destruct_object(this_object());
}

void receive_in(string input)
{
	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	switch(state) {
	case STATE_GETNAME:
		input = STRINGD->to_lower(input);

		if (strlen(input) && input[0] == '~') {
			silent = 1;
			input = input[1 ..];
		}

		if (!STRINGD->is_valid_username(input)) {
			send_out("That is not a valid username.\n");
			pop_state();
			return;
		} else if (!ACCOUNTD->query_is_registered(input)) {
			send_out("No such user.\n");
			pop_state();
			return;
		} else {
			state = STATE_CHKPASS;
			query_user()->set_mode(MODE_NOECHO);
			name = input;
			break;
		}
		break;

	case STATE_CHKPASS:
		send_out("\n");
		query_user()->set_mode(MODE_ECHO);
		password = input;

		if (!ACCOUNTD->query_is_registered(name)) {
			send_out("Whoops, that account no longer exists.\n");
			pop_state();
			return;
		} else if (!ACCOUNTD->authenticate(name, password)) {
			send_out("Password mismatch.\n");
			/* we will eventually want to ban IPs that */
			/* fail too much */
			query_user()->quit();
			return;
		} else if (BAND->query_is_user_banned(name)) {
			send_out("You are banned.\n");
			query_user()->quit();
			return;
		} else {
			if (TEXT_USERD->find_user(name)) {
				send_out("You are already logged in.\nDo you wish to disconnect your previous login? ");
				state = STATE_CHKDUPE;
				break;
			}

			query_user()->set_username(name);

			if (silent) {
				ACCOUNTD->set_account_property(name, "invisible", 1);
			}

			TEXT_SUBD->login_user(query_user());
			TEXT_SUBD->send_login_message(name);

			terminate_account_state();

			return;
		}
		break;

	case STATE_CHKDUPE:
		if (!ACCOUNTD->query_is_registered(name)) {
			send_out("Whoops, that account no longer exists.\n");
			pop_state();
			return;
		} else if (!ACCOUNTD->authenticate(name, password)) {
			send_out("Your password was just changed!\n");
			query_user()->quit();
			return;
		} else if (BAND->query_is_user_banned(name)) {
			send_out("Sorry, but you were just banned.\n");
			query_user()->quit();
			return;
		} else if (input == "yes") {
			object user;

			user = TEXT_USERD->find_user(name);

			if (user) {
				send_out("Evicting previous connection.\n");
				user->quit();
			} else {
				send_out("Your previous connection went away before I could evict it.\n");

				TEXT_SUBD->send_to_all_except(
					TEXT_SUBD->query_titled_name(name)
					+ " logs in.\n", ({ user }));
			}

			query_user()->set_username(name);
			TEXT_SUBD->login_user(query_user());
			terminate_account_state();
		} else {
			send_out("Ok then.\n");
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

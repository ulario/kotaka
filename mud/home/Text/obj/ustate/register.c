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
#include <kotaka/paths/text.h>
#include <kotaka/privilege.h>

#define STATE_GETUSERNAME 1
#define STATE_GETPASSWORD 2
#define STATE_CHKPASSWORD 3

inherit "/lib/string/validate";
inherit TEXT_LIB_USTATE;

string username;
string password;

int state;

int pending;

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
}

void set_username(string new_username)
{
	ACCESS_CHECK(TEXT());

	username = new_username;
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	if (query_user()->query_username()) {
		send_out("You are already logged in.\n");
		pop_state();
		return;
	}

	if (username) {
		state = STATE_GETPASSWORD;
		send_out("New password: ");
		query_user()->set_mode(MODE_NOECHO);
	} else {
		state = STATE_GETUSERNAME;
		send_out("New username: ");
	}
}

void end()
{
	destruct_object(this_object());
}

void receive_in(string input)
{
	ACCESS_CHECK(previous_object() == query_user());

	switch(state) {
	case STATE_GETUSERNAME:
		input = to_lower(input);

		if (!is_valid_username(input)) {
			send_out("That is not a valid username.\n");
			pop_state();
			return;
		} else if (ACCOUNTD->query_is_registered(input)) {
			send_out("That username is already taken.\n");
			pop_state();
			return;
		} else {
			state = STATE_GETPASSWORD;
			send_out("New password: ");
			query_user()->set_mode(MODE_NOECHO);
			username = input;
			return;
		}

	case STATE_GETPASSWORD:
		if (ACCOUNTD->query_is_registered(username)) {
			query_user()->set_mode(MODE_ECHO);
			send_out("\n");
			send_out("Oops...someone else just swiped the username you wanted.\n");
			query_user()->set_mode(MODE_ECHO);
			pop_state();
			return;
		} else if (BAND->query_is_user_banned(username)) {
			query_user()->set_mode(MODE_ECHO);
			send_out("\n");
			send_out("Your username was just banned!\n");
			send_out(BAND->query_ban_message(username));
			query_user()->quit("banned");
			return;
		} else {
			password = input;
			state = STATE_CHKPASSWORD;
			send_out("\nConfirm password: ");
			query_user()->set_mode(MODE_NOECHO);
			return;
		}

	case STATE_CHKPASSWORD:
		query_user()->set_mode(MODE_ECHO);
		send_out("\n");

		if (ACCOUNTD->query_is_registered(username)) {
			send_out("Oops...someone else just swiped the username you wanted.\n");
			pop_state();
			return;
		} else if (BAND->query_is_user_banned(username)) {
			send_out("Your username was just banned!\n");
			send_out(BAND->query_ban_message(username));
			query_user()->quit("banned");
			return;
		} else if (input != password) {
			send_out("Password mismatch.\n");
			pop_state();
			return;
		} else {
			object parent;

			ACCOUNTD->register_account(username, password);

			query_user()->login_user(username);

			ACCOUNTD->set_account_property(username, "channels", ({ "chat" }) );

			parent = query_parent();

			if (instanceof(parent, "login")) {
				object shell;

				shell = clone_object("shell");
				parent->push_state(shell);
				parent->collapse_state(shell);
			} else {
				pop_state();
			}
		}
	}
}

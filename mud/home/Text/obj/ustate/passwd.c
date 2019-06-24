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

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
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
	send_out("Please choose a password: ");
	query_user()->set_mode(MODE_NOECHO);
}

void end()
{
	destruct_object(this_object());
}

private int authorized()
{
	user = query_user();

	if (name != user->query_name()) {
		switch(TEXT_SUBD->query_user_class(name)) {
		case 4: /* owner */
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

	switch(state) {
	case STATE_GETPASS:
		send_out("\n");

		if (!ACCOUNTD->query_is_registered(name)) {
			query_user()->set_mode(MODE_ECHO);
			send_out("That account no longer exists.\n");
			pop_state();
			return;
		}

		if (!authorized()) {
			query_user()->set_mode(MODE_ECHO);
			send_out("You are no longer authorized to change that password.\n");
			pop_state();
			return;
		}

		password = input;
		state = STATE_CHKPASS;
		send_out("Please confirm your password: ");
		return;

	case STATE_CHKPASS:
		send_out("\n");
		query_user()->set_mode(MODE_ECHO);

		if (!ACCOUNTD->query_is_registered(name)) {
			send_out("That account no longer exists.\n");
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
	}
}

int forbid_log_inbound()
{
	return 1;
}

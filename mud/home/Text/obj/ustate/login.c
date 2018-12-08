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
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/account.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/privilege.h>

#define STATE_GETUSERNAME  1 /* asking for username */
#define STATE_GETPASSWORD  2 /* asking for password */
#define STATE_CHKDUPLICATE 3 /* checking to see if already logged in */
#define STATE_CHKNEWUSER   4 /* ask if they want to create a new account */
#define STATE_CHKPASSWORD  5 /* confirm password */

inherit "/lib/string/validate";
inherit TEXT_LIB_USTATE;

string username;
string password;
int silent;
int state;

int stopped;
int pending;
int persist;

private void prompt()
{
	switch(state) {
	case STATE_GETUSERNAME:
		send_out("Login: ");
		break;

	case STATE_GETPASSWORD:
		send_out("Password: ");
	}
}

private string get_ip()
{
	object conn;

	conn = query_user();

	while (conn && conn <- LIB_USER) {
		conn = conn->query_conn();
	}

	if (!conn) {
		LOGD->post_message("system", LOG_WARNING, "Bogus connection chain");

		return nil;
	}

	return query_ip_number(conn);
}

private string garbage(string input)
{
	if (strlen(input) >= 4 && input[0 .. 3] == "GET ") {
		return "http";
	}

	if (strlen(input) >= 1 && input[0] < ' ') {
		return "control";
	}

	return nil;
}

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	if (query_user()->query_name()) {
		send_out("You are already logged in.\n");
		pop_state();
		return;
	}

	send_out("Welcome to Ulario:\n\n");
	send_out("If you wish to connect as a guest, simply type enter.\n\n");

	state = STATE_GETUSERNAME;

	prompt();
}

void stop()
{
	stopped = 1;
}

void go()
{
	stopped = 0;

	if (pending) {
		pending = 0;

		prompt();
	}
}

void end()
{
	if (!persist) {
		destruct_object(this_object());
	}
}

void receive_in(string input)
{
	string garbage;

	ACCESS_CHECK(previous_object() == query_user());

	switch(state) {
	case STATE_GETUSERNAME:
		garbage = garbage(input);

		if (garbage) {
			string ip;

			ip = get_ip();

			LOGD->post_message("system", LOG_WARNING, "Banning " + ip + " for sending " + garbage + " garbage");

			BAND->ban_site(ip, ([
				"expire": time() + 30 * 24 * 60 * 60,
				"issuer": "Text",
				"message": "Sending garbage"
			]) );

			return;
		}

		input = to_lower(input);

		if (input == "") {
			object parent;

			parent = query_parent();

			if (!parent) {
				swap_state(clone_object("shell"));
			} else {
				ASSERT(parent <- "shell");
				pop_state();
			}
			return;
		}

		if (strlen(input) && input[0] == '~') {
			silent = 1;
			input = input[1 ..];
		}

		if (!is_valid_username(input)) {
			send_out("That is not a valid username.\n\n");
			break;
		}

		username = input;

		if (!ACCOUNTD->query_is_registered(username)) {
			if (BAND->query_is_user_banned(username)) {
				send_out("That username is banned.\n");
				query_user()->quit("banned");
				return;
			} else {
				send_out("No such username exists.\n");
				send_out("Create a new account? ");
				state = STATE_CHKNEWUSER;
				return;
			}
		}

		username = input;
		query_user()->set_mode(MODE_NOECHO);
		state = STATE_GETPASSWORD;
		break;

	case STATE_GETPASSWORD:
		password = input;
		query_user()->set_mode(MODE_ECHO);
		send_out("\n");

		if (ACCOUNTD->query_is_registered(username)) {
			if (!ACCOUNTD->authenticate(username, password)) {
				object user;
				string ip;

				send_out("Wrong password\n");

				persist = 1;

				ip = get_ip();

				query_user()->quit("badpass");

				"~/sys/faild"->strike(ip);

				destruct_object(this_object());

				return;
			}

			if (BAND->query_is_user_banned(username)) {
				string msg;

				send_out("You are banned.\n");

				msg = BAND->query_ban_message(username);

				if (msg) {
					send_out(msg + "\n");
				}

				query_user()->quit("banned");

				return;
			}

			if (TEXT_USERD->find_user(username)) {
				send_out("You are already logged in.\n");
				send_out("Do you wish to disconnect your previous login? ");
				state = STATE_CHKDUPLICATE;
				return;
			}

			if (silent) {
				ACCOUNTD->set_account_property(username, "invisible", 1);
			}

			query_user()->login_user(username);

			{
				object parent;

				parent = query_parent();

				if (!parent) {
					swap_state(clone_object("shell"));
				} else {
					ASSERT(parent <- "shell");
					pop_state();
				}
			}
			return;
		} else {
			send_out("This account was just deleted!\n");
			query_user()->query_quit("nuked");
			return;
		}

	case STATE_CHKDUPLICATE:
		if (!ACCOUNTD->query_is_registered(username)) {
			send_out("Your account was just deleted!\n");
			query_user()->query_quit("nuked");
			return;
		}

		if (!ACCOUNTD->authenticate(username, password)) {
			send_out("Your password was just changed!\n");
			query_user()->quit("badpass");
			return;
		}

		if (BAND->query_is_user_banned(username)) {
			string msg;

			send_out("You were just banned!\n");

			msg = BAND->query_ban_message(username);

			if (msg) {
				send_out(msg + "\n");
			}

			query_user()->quit("banned");
			return;
		}

		switch (to_lower(input)) {
		case "y":
		case "ye":
		case "yes":
			{
				object user;
				object conn;

				user = TEXT_USERD->find_user(username);

				if (user) {
					send_out("Evicting previous connection.\n");


					conn = query_user()->query_conn();
					
					while (conn && conn <- LIB_USER) {
						conn = conn->query_conn();
					}

					user->message("\n\nYou, or someone with your password,\nlogged into your account from " + query_ip_number(conn) + "\n");
					user->quit("bumped");
					ASSERT(!user);
				} else {
					send_out("Your previous connection went away before I could evict it.\n");

					TEXT_SUBD->send_to_all_except(
						TEXT_SUBD->query_titled_name(username)
						+ " logs in.\n", ({ user })
					);
				}

				query_user()->login_user(username);
				swap_state(clone_object("shell"));
				return;
			}

		case "n":
		case "no":
			query_user()->quit("quit");
			return;

		default:
			send_out("Yes or no please: ");
			return;
		}

	case STATE_CHKNEWUSER:
		switch (to_lower(input)) {
		case "y":
		case "ye":
		case "yes":
			{
				object register;

				state = STATE_GETUSERNAME;

				register = clone_object("register");
				register->set_username(username);
				push_state(register);
			}
			break;

		case "n":
		case "no":
			query_user()->quit("quit");
			return;

		default:
			send_out("Yes or no please: ");
			return;
		}
	}

	if (!stopped) {
		prompt();
	} else {
		pending = 1;
	}
}

#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <game/paths.h>

inherit GAME_LIB_USTATE;

#define STATE_GETNAME	1
#define STATE_GETPASS	2
#define STATE_CHKPASS	3
#define STATE_CHKDUPE	4

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
	ACCESS_CHECK(previous_object() == query_user());

	destruct_object(this_object());
}

void receive_in(string input)
{
	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	switch(state) {
	case STATE_GETNAME:
		input = STRINGD->to_lower(input);
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
		password = input;
		if (!ACCOUNTD->query_is_registered(name)) {
			send_out("Whoops, that account no longer exists.\n");
			query_user()->set_mode(MODE_ECHO);
			pop_state();
			return;
		} else if (!ACCOUNTD->authenticate(name, password)) {
			send_out("Password mismatch.\n");
			/* we will eventually want to ban IPs that */
			/* fail too much */
			query_user()->set_mode(MODE_ECHO);
			pop_state();
			return;
		} else {
			object user;
			/* todo: detect duplicates and prepare to */
			/* evict a linkdead user */

			user = GAME_USERD->find_user(name);

			if (user) {
				send_out("You are already logged in.\nDo you wish to disconnect your previous login? ");
				state = STATE_CHKDUPE;
				break;
			}

			if (GAME_USERD->query_is_guest(user)) {
				GAME_USERD->promote_guest(name, user);
			} else {
				GAME_USERD->add_user(name, user);
			}

			user->set_username(name);
			user->set_mode(MODE_ECHO);

			announce_login();
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
			send_out("Your password was just changed.\n");
			pop_state();
			return;
		} else if (input == "yes") {
			object user;

			user = GAME_USERD->find_user(name);

			if (user) {
				send_out("Evicting previous connection.\n");
				user->quit();
			} else {
				send_out("Your previous connection went away before I could evict it :P\n");
			}

			user = query_user();
			if (GAME_USERD->query_is_guest(user)) {
				GAME_USERD->promote_guest(name, user);
			} else {
				GAME_USERD->add_user(name, user);
			}

			user->set_username(name);
			user->set_mode(MODE_ECHO);

			announce_login();
			terminate_account_state();
			return;
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

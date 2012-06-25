#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <game/paths.h>

inherit GAME_LIB_USTATE;

#define STATE_GETNAME	1
#define STATE_GETPASS	2
#define STATE_CHKPASS	3

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
		send_out("Please choose a username: ");
		break;

	case STATE_GETPASS:
		send_out("Please choose a password: ");
		break;

	case STATE_CHKPASS:
		send_out("Please confirm your password: ");
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
		} else if (ACCOUNTD->query_is_registered(input)) {
			send_out("That name is already taken.\n");
			pop_state();
			return;
		} else {
			state = STATE_GETPASS;
			query_user()->set_mode(MODE_NOECHO);
			name = input;
			break;
		}
		break;

	case STATE_GETPASS:
		send_out("\n");
		if (ACCOUNTD->query_is_registered(name)) {
			send_out("Whoops, someone else just swiped the username you wanted.\n");
			query_user()->set_mode(MODE_ECHO);
			pop_state();
			return;
		} else if (BAND->query_is_banned(name)) {
			send_out("That name is banned.\n");
			query_user()->quit();
			return;
		} else {
			password = input;
			state = STATE_CHKPASS;
			break;
		}
		break;

	case STATE_CHKPASS:
		send_out("\n");
		if (ACCOUNTD->query_is_registered(name)) {
			send_out("Whoops, someone else just swiped the username you wanted.\n");
			query_user()->set_mode(MODE_ECHO);
			pop_state();
			return;
		} else if (BAND->query_is_banned(name)) {
			send_out("Whoops, the username you picked just got banned.\n");
			query_user()->quit();
			return;
		} else if (input != password) {
			send_out("Password mismatch.\n");
			query_user()->set_mode(MODE_ECHO);
			pop_state();
			return;
		} else {
			object user;
			
			user = query_user();
			ACCOUNTD->register_account(name, password);
			user->set_username(name);
			if (GAME_USERD->query_is_guest(user)) {
				GAME_USERD->promote_guest(name, user);
			} else {
				GAME_USERD->add_user(name, user);
			}
			user->set_mode(MODE_ECHO);

			GAME_SUBD->send_to_all_except(
				GAME_SUBD->titled_name(
					user->query_username(),
					user->query_class())
				+ " registers.\n", ({ user }));

			terminate_account_state();
			return;
		}
		break;
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}

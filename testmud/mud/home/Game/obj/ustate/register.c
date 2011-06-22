#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kernel/user.h>

#include <game/paths.h>

inherit LIB_USTATE;

int stopped;
int reading;

string username;
string password;
string email;
int un_invalid;
int pw_confirmed;

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
	if (!username) {
		send_out("Desired username: ");
	} else if (un_invalid) {
		send_out("Please choose another username: ");
	} else if (!password) {
		send_out("Password: ");
	} else if (!pw_confirmed) {
		send_out("Password (confirm): ");
	}
}

void set_username(string new_username)
{
	ACCESS_CHECK(!query_user());

	username = new_username;
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	send_out("New account registration.\n\n");
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

void end()
{
	ACCESS_CHECK(previous_object() == query_user());

	destruct_object(this_object());
}

private int username_valid()
{
	return STRINGD->regex_match(username, "[0-9a-zA-Z-_]+");
}

void receive_in(string input)
{
	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	if (!username || un_invalid) {
		username = input;
		un_invalid = 0;
		
		if (!username_valid()) {
			un_invalid = 1;
			send_out("Sorry, that is not a valid username.\nValid usernames consist only of\nletters, numbers, hyphens, and underscores.\n");
		} else if (BAND->query_is_banned(username)) {
			un_invalid = 1;
			send_out("Sorry, that username is currently banned.\n");
		} else if (ACCOUNTD->query_is_registered(username)) {
			un_invalid = 1;
			send_out("Sorry, that username is already taken.\n");
		} else {
			query_user()->set_mode(MODE_NOECHO);
		}
	} else if (!password) {
		send_out("\n");
		password = input;
	} else if (!pw_confirmed) {
		send_out("\n");
		query_user()->set_mode(MODE_ECHO);

		if (input == password) {
			if (BAND->query_is_banned(username)) {
				username = nil;
				un_invalid = 1;
				send_out("Looks like that username just got banned.\n");
			} else if (ACCOUNTD->query_is_registered(username)) {
				username = nil;
				un_invalid = 1;
				send_out("Looks like someone beat you to the punch.\n");
			} else {
				object user;

				send_out("Account registered.  Welcome to Ulario.\n");

				ACCOUNTD->register_account(
					username, password
				);

				user = query_user();
				user->set_name(username);

				if (GAME_USERD->query_is_guest(user)) {
					GAME_USERD->promote_guest(username, user);
				} else {
					GAME_USERD->add_user(username, user);
				}

				pop_state();

				return;
			}
		} else {
			password = nil;
			send_out("Those passwords don't match.  Try again\n");
		}
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}

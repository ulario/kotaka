#include <kotaka/paths.h>
#include <kernel/user.h>
#include <kotaka/privilege.h>

#include <game/paths.h>

inherit LIB_USTATE;

int stopped;
int reading;

string username;
object authd;

static void create(int clone)
{
	::create();
	authd = find_object("~/sys/authd");
}

static void destruct(int clone)
{
	::destruct();
}

private void prompt()
{
	if (!username) {
		send_out("Login: ");
	} else {
		send_out("Password: ");
	}
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

private int username_valid(string username)
{
	return STRINGD->regex_match(username, "[0-9a-zA-Z-_]+");
}

void receive_in(string input)
{
	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	if (username) {
		int uid;
		object shell;
		object parent;
		object user;

		if (!ACCOUNTD->query_is_registered(username)) {
			username = nil;
			send_out("Strange, that account has disappeared.\n");
		} else if (BAND->query_is_banned(username)) {
			send_out("Strange, that account has been banned.\n");
			query_user()->quit();
			return;
		} else if (!ACCOUNTD->authenticate(username, input)) {
			send_out("Wrong password.\n");
		} else {
			send_out("\nLogged in\n");

			user = query_user();
			user->set_mode(MODE_ECHO);

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
		if (!username_valid(input)) {
			send_out("Invalid username.\n");
		} else if (!ACCOUNTD->query_is_registered(input)) {
			send_out("No such account.\n");
		} else if (BAND->query_is_banned(input)) {
			send_out("That account is currently banned.");
			query_user()->quit();
			return;
		} else {
			username = input;
			query_user()->set_mode(MODE_NOECHO);
		}
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}

#include <kotaka/paths.h>
#include <kernel/user.h>
#include <kotaka/privilege.h>

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
		send_out("Username: ");
	} else {
		send_out("Password: ");
	}
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	send_out("Login\n");
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
		
		send_out("\nLogged in\n");
		query_user()->set_mode(MODE_ECHO);
		
		uid = "~/sys/accountd"->username_to_uid(username);
		
		query_user()->set_uid(uid);
		pop_state();

		return;
	} else {
		if (username_valid(input)) {
			username = input;
			query_user()->set_mode(MODE_NOECHO);
		} else {
			send_out("Invalid username.\n");
		}
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}

#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kernel/user.h>

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

static void begin()
{
	send_out("New account registration.\n\n");
}

static void stop()
{
	stopped = 1;
}

static void go()
{
	stopped = 0;

	if (!reading) {
		prompt();
	}
}

static void end()
{
	send_out("End.\n");

	destruct_object(this_object());
}

private int username_valid()
{
	return STRINGD->regex_match(username, "[0-9a-zA-Z-_]+");
}

private int username_available()
{
	/* todo:  develop a user management system */
	return random(2);
}

static void receive_in(string input)
{
	reading = 1;

	if (!username || un_invalid) {
		username = input;
		un_invalid = 0;
		
		if (!username_valid()) {
			un_invalid = 1;
			send_out("Sorry, that is not a valid username.\nValid usernames consist only of\nletters, numbers, hyphens, and underscores.\n");
		} else if (!username_available()) {
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
			send_out("Congratulations, you have successfully tested\nthe character registration shell.\n");
			
			query_user()->set_uid(1337);
			pop_ustate();

			return;
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

static void receive_out(string output)
{
	send_out("[outgoing] " + output);
}

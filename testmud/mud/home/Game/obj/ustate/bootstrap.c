#include <kotaka/paths.h>
#include <kotaka/privilege.h>

inherit LIB_USTATE;

int stopped;
int reading;
int strikes;
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
	send_out("UlarioMUD connection menu\n");
	send_out("-------------------------\n");
	send_out("1. login an existing account\n");
	send_out("2. register a new account\n");
	send_out("3. connect as a guest\n");
	send_out("4. help\n");
	send_out("5. disconnect\n");
	send_out("> ");
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());
}

void stop()
{
	ACCESS_CHECK(previous_object() == query_user());

	stopped = 1;
}

void pop(object state)
{
	ACCESS_CHECK(previous_object() == query_user());

	if (dead) {
		send_out("Dead bootstrap, ignoring pop.\n");
		return;
	}

	if (state <- "login" || state <- "register") {
		dead = 1;
		send_out("Bootstrap terminated, starting shell.\n");
		swap_state(clone_object("shell"));
		return;
	}
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

private void do_help()
{
	send_out(read_file("~/data/doc/guest_help"));
}

void receive_in(string input)
{
	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	switch(input) {
	case "1":
		send_out("Pushed login.\n");
		push_state(clone_object("login"));
		return;

	case "2":
		send_out("Pushed register.\n");
		push_state(clone_object("register"));
		return;

	case "3":
		swap_state(clone_object("shell"));
		return;

	case "4":
		do_help();
		break;

	case "5":
		send_out("Thanks for visiting.\n");
		query_user()->quit();
		return;

	default:
		strikes++;
		switch(strikes)
		{
		case 1: send_out("Please choose a number.\n"); break;
		case 2: send_out("Come back when you make up your mind.\n");
			query_user()->quit();
			return;
		}
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}

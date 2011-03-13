#include <kotaka/paths.h>

inherit LIB_USTATE;

int stopped;
int reading;
int strikes;

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

static void begin()
{
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
	destruct_object(this_object());
}

void authenticated()
{
}

private void do_help()
{
	send_out(read_file("~/data/doc/guest_help"));
}

static void receive_in(string input)
{
	reading = 1;

	switch(input) {
	case "1":
		push_state(clone_object("login"));
		return;

	case "2":
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

static void receive_out(string output)
{
	send_out("[outgoing] " + output);
}

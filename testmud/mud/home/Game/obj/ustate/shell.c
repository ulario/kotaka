#include <kernel/access.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <game/paths.h>

inherit GAME_LIB_USTATE;

int stopped;
int reading;

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
	string name;

	name = query_user()->query_username();

	if (!name) {
		name = "guest";
	}

	send_out("[" + name + "@ulario] ");
}

void begin()
{
	ACCESS_CHECK(previous_object() == query_user());

	send_out(read_file("~/data/doc/guest_welcome"));
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
	object user;
	string name;

	ACCESS_CHECK(previous_object() == query_user());

	user = query_user();
	name = titled_name(user->query_username(), user->query_class());

	send_out("Come back soon.\n");
	send_to_all_except(name + " dies and sinks into the ground.\n", ({ user }) );
}

void end()
{
	ACCESS_CHECK(previous_object() == query_user());

	destruct_object(this_object());
}

void receive_in(string input)
{
	string first;

	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	if (strlen(input) > 0) {
		switch(input[0]) {
		case '\'':
			input = input[1 ..];
			first = "say";
			break;
		case ':':
			input = input[1 ..];
			first = "emote";
			break;
		}
	}

	if (!first && !sscanf(input, "%s %s", first, input)) {
		first = input;
		input = "";
	}

	switch(first) {
	case "":
		break;
	default:
		if (!BIND->execute_command(first, input)) {
			send_out(first + ": command not recognized.\n");
		}
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}

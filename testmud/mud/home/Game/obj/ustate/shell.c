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

void do_help()
{
	object pager;

	pager = clone_object("~Kotaka/obj/ustate/page");

	pager->set_text(read_file("~/data/help/player/index.hlp"));

	push_state(pager);
}

void do_emote(string args)
{
	object user;
	string name;

	user = query_user();

	if (user->query_class() < 1) {
		send_out("You do not have sufficient access rights to emote.\n");
		return;
	}

	args = STRINGD->trim_whitespace(args);

	if (args == "") {
		send_out("Cat got your tongue?\n");
		return;
	}

	name = titled_name(user->query_username(), user->query_class());

	send_out("You " + args + "\n");
	send_to_all_except(name + " " + args + "\n", ({ user }) );
}

void do_say(string args)
{
	object user;
	string name;

	user = query_user();

	if (user->query_class() < 1) {
		send_out("You do not have sufficient access rights to speak.\n");
		return;
	}

	args = STRINGD->trim_whitespace(args);

	if (args == "") {
		send_out("Cat got your tongue?\n");
		return;
	}

	name = titled_name(user->query_username(), user->query_class());

	send_out("You say: " + args + "\n");
	send_to_all_except(name + " says: " + args + "\n", ({ user }) );
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
	case "login":
		push_state(clone_object("login"));
		break;
	case "register":
		push_state(clone_object("register"));
		break;
	case "help":
		do_help();
		break;
	case "play":
		push_state(clone_object("play"));
		break;
	case "quit":
		query_user()->quit();
		return;
	case "say":
		do_say(input);
		break;
	case "emote":
		do_emote(input);
		break;
	case "krecompile":
		OBJECTD->klib_recompile();
		break;
	case "recompile":
		OBJECTD->global_recompile();
		break;
	case "trecompile":
		OBJECTD->klib_recompile();
		OBJECTD->global_recompile();
		break;
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

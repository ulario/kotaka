#include <kernel/access.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <game/paths.h>

inherit LIB_USTATE;

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

	name = query_user()->query_name();

	if (!name) {
		name = "(anonymous)";
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
	ACCESS_CHECK(previous_object() == query_user());

	send_out("Shell end.\n");
}

void end()
{
	ACCESS_CHECK(previous_object() == query_user());

	destruct_object(this_object());
}

private void scan_world()
{
	object root;
	object *kids;

	int i;
	int sz;

	root = find_object(ROOT);

	kids = root->query_inventory();
	sz = sizeof(kids);

	for (i = 0; i < sz; i++) {
		object obj;
		float x, y, z;

		obj = kids[i];

		x = obj->query_property("position:x");
		y = obj->query_property("position:y");
		z = obj->query_property("position:z");

		send_out("Object " + object_name(obj) + " located at (" +
			x + ", " + y + ", " + z + ")\n");
	}
}

void do_who()
{
	object *users;
	string **lists;
	int sz, i;

	lists = ({ ({ }), ({ }), ({ }) });

	send_out("User list\n");
	send_out("---------\n");

	users = GAME_USERD->query_users();
	sz = sizeof(users);

	for (i = 0; i < sz; i++) {
		lists[users[i]->query_class() - 1]
			+= ({ users[i]->query_name() });
	}

	for (i = 0; i < 3; i++) {
		if (sizeof(lists[i])) {
			int j;
			string *list;
			
			list = lists[i];
			sz = sizeof(list);

			switch(i) {
			case 0: send_out("Players:\n"); break;
			case 1: send_out("Wizards:\n"); break;
			case 2: send_out("Administrators:\n"); break;
			}

			for (j = 0; j < sz; j++) {
				send_out(list[j] + "\n");
			}
		}
	}
}

void do_help()
{
	object pager;

	pager = clone_object("~Kotaka/obj/ustate/page");

	pager->set_text(read_file("~/data/help/player/index.hlp"));

	push_state(pager);
}

private void send_to_all(string phrase)
{
	int sz;
	object *users;

	users = GAME_USERD->query_users();

	for (sz = sizeof(users) - 1; sz >= 0; sz--) {
		users[sz]->message(phrase);
	}
}

private void send_to_all_except(string phrase, object *exceptions)
{
	int sz;
	object *users;

	users = GAME_USERD->query_users() - exceptions;

	for (sz = sizeof(users) - 1; sz >= 0; sz--) {
		users[sz]->message(phrase);
	}
}

private string titled_name(string name, int class)
{
	if (name) {
		STRINGD->to_title(name);
	} else {
		name = "(anonymous)";
	}

	switch(class) {
	case 0:
		name = "\033[1;34m" + name + "\033[0m";
		break;
	case 1:
		name = "\033[1;32m" + name + "\033[0m";
		break;
	case 2:
		name = "\033[1;33m" + name + "\033[0m";
		break;
	case 3:
		name = "\033[1;31m" + name + "\033[0m";
		break;
	}

	return name;
}

void do_say(string args)
{
	object user;
	string name;

	args = STRINGD->trim_whitespace(args);

	if (args == "") {
		send_out("Cat got your tongue?\n");
		return;
	}

	user = query_user();
	name = titled_name(user->query_name(), user->query_class());

	send_out("You say: " + args + "\n");
	send_to_all_except(name + " says: " + args + "\n", ({ user }) );
}

private void do_kick(string args)
{
	object user;
	object turkey;
	string kicker_name;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("You do not have sufficient access rights to kick someone from the mud.\n");
		return;
	}

	if (args == "") {
		send_out("Who do you wish to kick?\n");
		return;
	}

	turkey = GAME_USERD->query_user(args);

	if (!turkey) {
		send_out("No such user is on the mud.\n");
		return;
	}

	kicker_name = titled_name(user->query_name(), user->query_class());

	user->message("You kick " + args + " from the mud.\n");
	turkey->message(kicker_name + " kicks you from the mud!\n");
	send_to_all_except(kicker_name + " kicks " + args + "from the mud!\n", ({ turkey, query_user() }) );

	turkey->quit();
}

private void do_ban(string args)
{
	object turkey;
	string kicker_name;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("You do not have sufficient access rights to ban someone from the mud.\n");
		return;
	}

	if (args == "") {
		send_out("Who do you wish to ban?\n");
		return;
	}

	if (args == "admin") {
		send_out("You cannot kick admin.\n");
		return;
	}

	if (args == user->query_name()) {
		send_out("You cannot kick yourself.\n");
		return;
	}

	if (BAND->query_is_banned(args)) {
		send_out("That user is already banned.\n");
		return;
	}

	BAND->ban_username(args);

	turkey = GAME_USERD->query_user(args);
	kicker_name = titled_name(user->query_name(), user->query_class());

	user->message("You ban " + args + " from the mud.\n");

	send_to_all_except(args + " has been banned from the mud by " + kicker_name + ".\n", ({ turkey, query_user() }) );


	if (turkey) {
		turkey->message("You have been banned from the mud by " + kicker_name + "!\n");
		turkey->quit();
	}
}

private void do_unban(string args)
{
	string kicker_name;
	object user;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("You do not have sufficient access rights to ban someone from the mud.");
		return;
	}

	if (args == "") {
		send_out("Who do you wish to ban?\n");
		return;
	}

	if (args == "admin") {
		send_out("You cannot ban admin.\n");
		return;
	}

	if (args == user->query_name()) {
		send_out("You cannot ban yourself.\n");
		return;
	}

	if (!BAND->query_is_banned(args)) {
		send_out("That user is not currently banned.\n");
		return;
	}

	kicker_name = titled_name(user->query_name(), user->query_class());

	user->message("You unban " + args + " from the mud.\n");

	send_to_all_except(args + " has been unbanned from the mud by " + kicker_name + ".\n", ({ user }) );

	BAND->unban_username(args);
}

void receive_in(string input)
{
	string first;

	ACCESS_CHECK(previous_object() == query_user());

	reading = 1;

	if (!sscanf(input, "%s %s", first, input)) {
		first = input;
		input = "";
	}

	switch(first) {
	case "who":
		do_who();
		break;
	case "help":
		do_help();
		break;
	case "play":
		push_state(clone_object("play"));
		break;
	case "statwatch":
		push_state(clone_object("status"));
		break;
	case "reloadhelp":
		GAME_HELPD->load();
		break;
	case "worldscan":
		scan_world();
		break;
	case "quit":
		query_user()->quit();
		return;
	case "login":
		push_state(clone_object("login"));
		break;
	case "unban":
		do_unban(input);
		break;
	case "ban":
		do_ban(input);
		break;
	case "kick":
		do_kick(input);
		break;
	case "say":
		do_say(input);
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
	case "register":
		push_state(clone_object("register"));
		break;
	case "":
		break;
	default:
		send_out(first + ": command not recognized.\n");
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}

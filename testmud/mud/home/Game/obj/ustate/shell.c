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
		send_out("[\033[1;30m(anonymous)\033[0m@ulario] ");
		return;
	}

	switch(query_user()->query_class()) {
	case 0: send_out("[\033[1;34m"); break;
	case 1: send_out("[\033[1;32m"); break;
	case 2: send_out("[\033[1;33m"); break;
	case 3: send_out("[\033[1;31m"); break;
	}

	send_out(query_user()->query_name() + "\033[0m@ulario] ");
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

	lists = allocate(4);

	send_out("User list");
	send_out("---------");

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
			case 0: send_out("Players:\n");
			case 1: send_out("Wizards:\n");
			case 2: send_out("Administrators:\n");
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

void do_say(string args)
{
	object user;
	string name;
	object *users;
	string phrase;
	int class;
	int sz;

	user = query_user();
	name = user->query_name();

	if (class = user->query_class() == 0) {
		name = "(anonymous)";
	}

	args = STRINGD->trim_whitespace(args);

	if (args == "") {
		send_out("Cat got your tongue?\n");
		return;
	}

	name = STRINGD->to_title(name);

	switch(user->query_class()) {
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

	users = GAME_USERD->query_users() + GAME_USERD->query_guests();
	users -= ({ user });

	user->message("You say: " + args + "\n");

	for (sz = sizeof(users) - 1; sz >= 0; sz--) {
		users[sz]->message(name + " says: " + args + "\n");
	}
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

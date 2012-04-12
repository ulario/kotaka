#include <kotaka/paths.h>
#include <game/paths.h>

/* overrides LIB_USTATE to provide auditing */

inherit LIB_USTATE;

static void send_to_all(string phrase)
{
	int sz;
	object *users;

	users = GAME_USERD->query_users();

	for (sz = sizeof(users) - 1; sz >= 0; sz--) {
		users[sz]->message(phrase);
	}
}

static void send_to_all_except(string phrase, object *exceptions)
{
	int sz;
	object *users;

	users = GAME_USERD->query_users() - exceptions;

	for (sz = sizeof(users) - 1; sz >= 0; sz--) {
		users[sz]->message(phrase);
	}
}

static string titled_name(string name, int class)
{
	if (name) {
		STRINGD->to_title(name);
	} else {
		name = "guest";
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

static void terminate_account_state()
{
	object parent;

	parent = query_parent();
	
	send_out(object_name(parent) + " is the parent ustate\n");

	if (parent <- "~/obj/ustate/start") {
		send_out("Parent is the bootstrap\n");
		parent->swap_state(clone_object("shell"));
	} else {
		send_out("Parent is not the bootstrap\n");
		pop_state();
	}
}

static void announce_register()
{
	send_out(
		titled_name(
			query_user()->query_username(),
			query_user()->query_class()
		) + " is born.\n");
}

static void announce_login()
{
	send_out(
		titled_name(
			query_user()->query_username(),
			query_user()->query_class()
		) + " returns from the dead.\n");
}

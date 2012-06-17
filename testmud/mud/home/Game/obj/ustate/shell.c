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
			+= ({ users[i]->query_username() });
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

void do_alist()
{
	string *users;
	object user;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("You do not have sufficient access rights to list accounts.\n");
		return;
	}

	users = ACCOUNTD->query_accounts();

	send_out("Users: " + implode(users, ", ") + "\n");
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

	turkey = GAME_USERD->find_user(args);

	if (!turkey) {
		send_out("No such user is on the mud.\n");
		return;
	}

	kicker_name = titled_name(user->query_username(), user->query_class());

	user->message("You kick " + args + " from the mud.\n");
	turkey->message(kicker_name + " kicks you from the mud!\n");
	send_to_all_except(kicker_name + " kicks " + args + "from the mud!\n", ({ turkey, query_user() }) );

	turkey->quit();
}

private void do_nuke(string args)
{
	object turkey;
	string kicker_name;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("You do not have sufficient access rights to nuke someone from the mud.\n");
		return;
	}

	if (args == "") {
		send_out("Who do you wish to nuke?\n");
		return;
	}

	if (args == user->query_username()) {
		send_out("You cannot nuke yourself.\n");
		return;
	}

	if (args == "admin") {
		send_out("You cannot nuke admin.\n");
		return;
	}

	if (!ACCOUNTD->query_is_registered(args)) {
		send_out("There is no such user.\n");
		return;
	}

	ACCOUNTD->unregister_account(args);

	turkey = GAME_USERD->find_user(args);
	kicker_name = titled_name(user->query_username(), user->query_class());

	user->message("You nuked " + args + " from the mud.\n");

	send_to_all_except(args + " has been nuked from the mud by " + kicker_name + ".\n", ({ turkey, query_user() }) );

	if (turkey) {
		turkey->message("You have been nuked from the mud by " + kicker_name + "!\n");
		turkey->quit();
	}
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

	if (args == user->query_username()) {
		send_out("You cannot kick yourself.\n");
		return;
	}

	if (args == "admin") {
		send_out("You cannot ban admin.\n");
		return;
	}

	if (args == user->query_username()) {
		send_out("You cannot ban yourself.\n");
		return;
	}

	if (BAND->query_is_banned(args)) {
		send_out("That user is already banned.\n");
		return;
	}

	BAND->ban_username(args);

	turkey = GAME_USERD->find_user(args);
	kicker_name = titled_name(user->query_username(), user->query_class());

	user->message("You ban " + args + " from the mud.\n");

	send_to_all_except(args + " has been banned from the mud by " + kicker_name + ".\n", ({ turkey, query_user() }) );

	if (turkey) {
		turkey->message("You have been banned from the mud by " + kicker_name + "!\n");
		turkey->quit();
	}
}

private void do_trace()
{
	object conn;

	send_out("Connection chain trace:\n");

	conn = query_user();

	while (conn) {
		send_out(object_name(conn) + "\n");

		if (conn <- LIB_USER) {
			conn = conn->query_conn();
		} else {
			break;
		}
	}
}

private void do_unban(string args)
{
	string kicker_name;
	object user;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("You do not have sufficient access rights to unban someone from the mud.");
		return;
	}

	if (args == "") {
		send_out("Who do you wish to unban?\n");
		return;
	}

	if (!BAND->query_is_banned(args)) {
		send_out("That user is not currently banned.\n");
		return;
	}

	kicker_name = titled_name(user->query_username(), user->query_class());
	user->message("You unban " + args + " from the mud.\n");
	send_to_all_except(args + " has been unbanned from the mud by " + kicker_name + ".\n", ({ user }) );
	BAND->unban_username(args);
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
	case "trace":
		do_trace();
		break;
	case "login":
		push_state(clone_object("login"));
		break;
	case "register":
		push_state(clone_object("register"));
		break;
	case "who":
		do_who();
		break;
	case "help":
		do_help();
		break;
	case "alist":
		do_alist();
		break;
	case "play":
		push_state(clone_object("play"));
		break;
	case "quit":
		query_user()->quit();
		return;
	case "unban":
		do_unban(input);
		break;
	case "nuke":
		do_nuke(input);
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
		send_out(first + ": command not recognized.\n");
	}

	reading = 0;

	if (!stopped) {
		prompt();
	}
}

#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	object turkey;
	object user;
	string kicker_name;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("Only an administrator can ban someone.\n");
		return;
	}

	if (args == "") {
		send_out("Who do you wish to ban?\n");
		return;
	}

	if (args == user->query_username()) {
		send_out("You cannot ban yourself.\n");
		return;
	}

	if (args == "admin") {
		send_out("You cannot ban admin.\n");
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

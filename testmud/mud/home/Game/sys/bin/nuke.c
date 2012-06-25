#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

private void do_nuke(string args)
{
	object turkey;
	object user;
	string kicker_name;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("Only an administrator can nuke someone.\n");
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
	kicker_name = SUBD->titled_name(user->query_username(), user->query_class());

	user->message("You nuked " + args + " from the mud.\n");

	SUBD->send_to_all_except(args + " has been nuked from the mud by " + kicker_name + "!\n", ({ turkey, query_user() }) );

	if (turkey) {
		turkey->message("You have been nuked from the mud by " + kicker_name + "!\n");
		turkey->quit();
	}
}

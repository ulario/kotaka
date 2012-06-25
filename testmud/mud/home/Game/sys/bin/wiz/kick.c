#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	object user;
	object turkey;
	string kicker_name;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("Only a wizard can kick someone.\n");
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

	kicker_name = GAME_SUBD->titled_name(user->query_username(), user->query_class());

	user->message("You kick " + args + " from the mud.\n");
	turkey->message(kicker_name + " kicks you from the mud!\n");
	GAME_SUBD->send_to_all_except(kicker_name + " kicks " + args + "from the mud!\n", ({ turkey, query_user() }) );

	turkey->quit();
}

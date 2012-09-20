#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	object turkey;
	object user;
	string kicker_name;
	string turkey_name;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("Only a wizard or an administrator can ban someone.\n");
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

	switch("~/sys/subd"->query_user_class(args)) {
	case 3: /* administrator.  Only the mud owner can ban them */
		if (user->query_username() != "admin") {
			send_out("Only the mud owner can ban an administrator.");
			return;
		}
		break;
	case 2: /* wizard.  Only an administrator can ban them */
		if (user->query_class() < 3) {
			send_out("Only an administrator can ban a wizard.");
			return;
		}
		break;
	default:
		if (user->query_class() < 2) {
			send_out("Only a wizard can ban someone.");
			return;
		}
		break;
	}

	BAND->ban_username(args);

	turkey = GAME_USERD->find_user(args);

	kicker_name = GAME_SUBD->titled_name(user->query_username(), user->query_class());
	turkey_name = GAME_SUBD->titled_name(args, "~/sys/subd"->query_user_class(args));

	user->message("You ban " + turkey_name + " from the mud.\n");

	GAME_SUBD->send_to_all_except(kicker_name + " banned " + turkey_name + " from the mud.\n", ({ turkey, query_user() }) );

	if (turkey) {
		turkey->message("You have been banned from the mud by " + kicker_name + ".\n");
		turkey->quit();
		GAME_SUBD->send_to_all_except(kicker_name + " kicked " + turkey_name + " from the mud.\n", ({ turkey, query_user() }) );
	}
}

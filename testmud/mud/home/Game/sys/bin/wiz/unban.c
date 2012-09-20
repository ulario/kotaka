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
		send_out("Only a wizard or an administrator can unban someone.\n");
		return;
	}

	if (args == "") {
		send_out("Who do you wish to unban?\n");
		return;
	}

	if (!BAND->query_is_banned(args)) {
		send_out("That user is not banned.\n");
		return;
	}

	switch("~/sys/subd"->query_user_class(args)) {
	case 3: /* administrator.  Only the mud owner can ban them */
		if (user->query_username() != "admin") {
			send_out("Only the mud owner can unban an administrator.");
			return;
		}
		break;
	case 2: /* wizard.  Only an administrator can ban them */
		if (user->query_class() < 3) {
			send_out("Only an administrator can unban a wizard.");
			return;
		}
		break;
	default:
		if (user->query_class() < 2) {
			send_out("Only a wizard can unban someone.");
			return;
		}
		break;
	}

	BAND->unban_username(args);

	kicker_name = GAME_SUBD->titled_name(user->query_username(), user->query_class());
	turkey_name = GAME_SUBD->titled_name(args, "~/sys/subd"->query_user_class(args));

	user->message("You unban " + turkey_name + " from the mud.\n");

	GAME_SUBD->send_to_all_except(kicker_name + " unbanned " + turkey_name + " from the mud.\n", ({ turkey, query_user() }) );
}

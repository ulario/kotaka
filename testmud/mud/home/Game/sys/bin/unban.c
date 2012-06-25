#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	string kicker_name;
	object user;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("Only an administrator can unban someone.\n");
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

	kicker_name = GAME_SUBD->titled_name(user->query_username(), user->query_class());
	user->message("You unban " + args + " from the mud.\n");
	GAME_SUBD->send_to_all_except(args + " has been unbanned from the mud by " + kicker_name + ".\n", ({ user }) );
	BAND->unban_username(args);
}

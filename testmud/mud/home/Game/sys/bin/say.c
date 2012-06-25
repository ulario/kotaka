#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
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

	name = GAME_SUBD->titled_name(user->query_username(), user->query_class());

	send_out("You say: " + args + "\n");
	GAME_SUBD->send_to_all_except(name + " says: " + args + "\n", ({ user }) );
}

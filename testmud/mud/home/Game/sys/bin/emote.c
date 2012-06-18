#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
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

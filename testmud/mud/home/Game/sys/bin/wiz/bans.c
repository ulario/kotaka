#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	string *users;
	object user;

	user = query_user();

	if (user->query_class() < 3) {
		send_out("You do not have sufficient access rights to list bans.\n");
		return;
	}

	users = BAND->query_username_bans();
	if (sizeof(users)) {
		send_out("Banned users: " + implode(users, ", ") + "\n");
	} else {
		send_out("Tnere are no banned users.\n");
	}
}

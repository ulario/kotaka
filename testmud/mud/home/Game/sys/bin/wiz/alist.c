#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
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

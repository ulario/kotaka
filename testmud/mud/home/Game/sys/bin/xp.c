#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	string name;
	mixed total;

	name = query_user()->query_name();

	if (!name) {
		send_out("You aren't logged in.\n");
		return;
	}

	total = ACCOUNTD->query_account_property(name, "xp");

	if (total) {
		if (total > 0) {
			send_out("You have " + total + " XP :)\n");
		} else {
			send_out("You are " + -total + " XP in debt! >:(\n");
		}
	} else {
		send_out("You have no XP :(\n");
	}
}

#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	string user;
	int amount;
	mixed total;

	if (query_user()->query_class() < 2) {
		send_out("Only a wizard can award XP.\n");
		return;
	}

	if (sscanf(args, "%s %d", user, amount) != 2) {
		send_out("Usage: award user amount.\n");
		return;
	}

	if (!ACCOUNTD->query_is_registered(user)) {
		send_out("No such user.\n");
		return;
	}

	total = ACCOUNTD->query_account_property(user, "xp");

	if (!total) {
		total = 0;
	}

	total += amount;

	if (total) {
		ACCOUNTD->set_account_property(user, "xp", total);
	} else {
		ACCOUNTD->set_account_property(user, "xp", nil);
	}
}

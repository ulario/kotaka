#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	int count;

	if (query_user()->query_class() < 1) {
		send_out("You do not have sufficient access rights to set off a clone bomb.\n");
		return;
	}

	if (!sscanf(args, "%d", count)) {
		send_out("I need a number please.\n");
		return;
	}

	GAME_TESTD->ignite(count);
}

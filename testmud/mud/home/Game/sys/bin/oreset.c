#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	object user;

	user = query_user();

	if (user->query_class() < 1) {
		send_out("You do not have sufficient access rights to reset the object manager.\n");
		return;
	}

	OBJECTD->full_reset();
}

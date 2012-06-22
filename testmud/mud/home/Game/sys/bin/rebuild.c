#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	object user;

	user = query_user();

	if (user->query_class() < 2) {
		send_out("You do not have sufficient access rights to rebuild.\n");
		return;
	}

	OBJECTD->klib_recompile();
	OBJECTD->global_recompile();
}

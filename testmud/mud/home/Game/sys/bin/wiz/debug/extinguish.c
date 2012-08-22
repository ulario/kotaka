#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	int count;

	if (query_user()->query_class() < 1) {
		send_out("You do not have sufficient access rights to extinguish a clone bomb.\n");
		return;
	}

	"~System/sys/extinguishd"->extinguish(USR_DIR + "/Game/obj/bomb");
}

#include <kotaka/paths.h>
#include <game/paths.h>
#include <status.h>

inherit LIB_WIZBIN;

void main(string args)
{
	mixed *st;
	object oinfo;
	int oindex;
	int count;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to do a clone check.\n");
		return;
	}

	st = status(args);

	if (!st) {
		send_out("No such object.\n");
		return;
	}

	oinfo = OBJECTD->query_object_info(st[O_INDEX]);

	send_out("There are " + oinfo->query_clone_count() + " clones.\n");
}

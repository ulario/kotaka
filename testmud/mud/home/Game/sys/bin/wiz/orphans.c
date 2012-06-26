#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	object arr;
	int sz, i;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to list orphans.\n");
		return;
	}

	arr = OBJECTD->query_orphans();

	if (!(sz = arr->get_size())) {
		send_out("There are no orphaned objects.\n");
		return;
	}

	for (i = 0; i < sz; i++) {
		send_out(arr->get_element(i) + "\n");
	}
}

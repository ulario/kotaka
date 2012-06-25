#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	int count;

	if (query_user()->query_class() < 3) {
		send_out("You do not have sufficient access rights to dump the mud.\n");
		return;
	}

	KERNELD->dump_state();
}

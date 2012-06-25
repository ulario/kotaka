#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	int count;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to swap out the mud.\n");
		return;
	}

	KERNELD->swapout();
}

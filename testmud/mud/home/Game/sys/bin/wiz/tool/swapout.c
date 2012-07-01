#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_WIZBIN;

void main(string args)
{
	int count;

	if (query_user()->query_class() < 2) {
		send_out("You do not have sufficient access rights to swap out the mud.\n");
		return;
	}

	proxy_call("swapout");
}

#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_WIZBIN;

void main(string args)
{
	int count;

	if (query_user()->query_class() < 3) {
		send_out("You do not have sufficient access rights to dump the mud.\n");
		return;
	}

	proxy_call("dump_state");
}

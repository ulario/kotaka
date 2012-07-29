#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_WIZBIN;

void main(string args)
{
	int count;

	if (query_user()->query_class() < 3) {
		send_out("You do not have sufficient access rights to tune the maze test.\n");
		return;
	}

	"~/sys/mazed"->set_size_goal((int)args);
}

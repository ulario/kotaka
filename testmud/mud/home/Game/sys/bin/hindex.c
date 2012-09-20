#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	mapping index;

	index = HELPD->query_index(args);

	if (index) {
		send_out("Index:\n" + STRINGD->hybrid_sprint(index) + "\n");
	} else {
		send_out("No such category.\n");
		return;
	}
}

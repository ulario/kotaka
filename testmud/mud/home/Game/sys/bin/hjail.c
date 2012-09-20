#include <kotaka/paths.h>
#include <game/paths.h>
#include <kotaka/assert.h>

inherit LIB_BIN;

void main(string args)
{
	string root;
	mapping index;

	if (!sscanf(args, "%s %s", root, args)) {
		root = "";
	}

	index = HELPD->query_index(root);
	ASSERT(index);

	if (index[args]) {
		send_out("Listings:\n" + STRINGD->hybrid_sprint(index[args]) + "\n");
	} else {
		send_out("No such entry in index.\n");
		return;
	}
}

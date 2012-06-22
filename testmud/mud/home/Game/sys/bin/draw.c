#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	object root;
	object *contents;
	object living;
	int sz;

	root = find_object(ROOT);

	contents = root->query_inventory();

	send_out("Look test:\n\n");

	if (sz = sizeof(contents)) {
		send_out("~/sys/subd"->draw_look(
			contents[random(sz)]
		));
	} else {
		send_out("~/sys/subd"->draw_look(nil));
	}

}

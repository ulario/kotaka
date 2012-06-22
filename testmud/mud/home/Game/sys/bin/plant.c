#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	object seed;
	object root;

	seed = clone_object("~/obj/object");

	seed->move(root = find_object(GAME_ROOT));

	seed->set_x_position((float)random(9));
	seed->set_y_position((float)random(9));

	send_out("A seed has been planted.\n");
	send_out("Root inventory is now:\n");
	send_out(STRINGD->tree_sprint(root->query_inventory()) + "\n");
}

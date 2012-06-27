#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	object seed;
	object root;
	object user;
	string sprint;

	user = query_user();

	if (user->query_class() < 1) {
		send_out("You do not have sufficient access rights to plant.\n");
		return;
	}

	switch(random(3)) {
	case 0:
		send_out("A placid leaf muncher is born.\n");
		seed = clone_object("~/obj/deer");
		break;
	case 1:
		send_out("A fierce spirited wolf is born.\n");
		seed = clone_object("~/obj/wolf");
		break;
	case 2:
		send_out("A boring rock is conjured.\n");
		seed = clone_object("~/obj/rock");
		break;
	}

	seed->move(root = find_object(GAME_ROOT));

	seed->set_x_position((float)random(9));
	seed->set_y_position((float)random(9));
}

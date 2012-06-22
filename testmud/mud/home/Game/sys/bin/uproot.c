#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	object *seeds;
	object root;
	int sz;
	object user;

	user = query_user();

	if (user->query_class() < 1) {
		send_out("You do not have sufficient access rights to uproot.\n");
		return;
	}

	seeds = (root = find_object(GAME_ROOT))->query_inventory();

	if ((sz = sizeof(seeds)) > 0) {
		destruct_object(seeds[random(sz)]);
		send_out("A seedling has been uprooted.\n");
	} else {
		send_out("There were no seedlings to uproot.\n");
	}
	send_out("Root inventory is now:\n");
	send_out(STRINGD->tree_sprint(root->query_inventory()) + "\n");
}

#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_VERB;

void main(object actor, string args)
{
	object rock;
	mixed inv;
	int sz;

	switch(actor->query_id_base()) {
	case "rock":
		send_out("Rocks don't have hands.\n");
		break;
	case "wolf":
	case "deer":
		inv = actor->query_property("holding");

		if (!inv || map_sizeof(inv) == 0) {
			send_out("You are not holding any rocks");
		}

		inv = map_indices(inv);
		sz = sizeof(inv);
		destruct_object(inv[random(sz)]);

		send_out("You drop a rock.\n");
	}
}

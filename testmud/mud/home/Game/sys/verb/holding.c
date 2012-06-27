#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_VERB;

void main(object actor, string args)
{
	object rock;
	mapping inv;

	switch(actor->query_id_base()) {
	case "rock":
		send_out("Rocks don't have hands.\n");
		break;
	case "wolf":
	case "deer":
		inv = actor->query_property("holding");
		send_out("You have in your hands " + map_sizeof(inv) + " rocks.\n");
	}
}

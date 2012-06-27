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
		send_out("You pick up a rock.\n");
		rock = clone_object("~/obj/rock");
		rock->move(actor);

		inv = actor->query_property("holding");
		if (!inv) {
			inv = ([ ]);
		}
		inv[rock] = 1;
		actor->set_property("holding", inv);
	}
}

#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_VERB;

void main(object actor, string args)
{
	GAME_SUBD->draw_look(actor, 0);
}

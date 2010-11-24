#include <kotaka/paths.h>
#include <game/paths.h>

inherit GAME_LIB_OBJECT;
/* bow */

/* used to shoot arrows */
/* can shoot up to 3 squares */
/* cannot be used to hit wolves */

/* whittled from wood */
static void create(int clone)
{
	if (clone) {
		::create();
	}
}

static void destruct(int clone)
{
	if (clone) {
		::destruct();
	}
}

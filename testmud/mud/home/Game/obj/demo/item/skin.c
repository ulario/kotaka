#include <kotaka/paths.h>
#include <game/paths.h>

inherit GAME_LIB_OBJECT;
/* skin */
/* harvested from dead animals */
/* can be used to make armor */
/* can be eaten by wolves */
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

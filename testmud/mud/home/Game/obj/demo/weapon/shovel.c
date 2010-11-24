#include <kotaka/paths.h>
#include <game/paths.h>

inherit GAME_LIB_OBJECT;
/* Shovel */

/* digs holes */
/* bash/cut weapon */
/* forged from iron and wood */
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

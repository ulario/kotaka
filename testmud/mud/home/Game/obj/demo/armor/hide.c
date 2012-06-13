#include <kotaka/paths.h>
#include <game/paths.h>

inherit GAME_LIB_OBJECT;
/* hide armor */

/* assembled from skins, absorbs damage */
/* degrades from usage */

/*

damage absorption:

damage cancelled to wearer = damage dealt * condition

*/

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

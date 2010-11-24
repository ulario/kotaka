#include <kotaka/paths.h>
#include <game/paths.h>

inherit GAME_LIB_OBJECT;

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

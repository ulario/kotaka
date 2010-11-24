#include <kotaka/paths.h>
#include <kotaka/bigstruct.h>
#include <kotaka/log.h>

inherit SECOND_AUTO;
inherit LIB_BIGSTRUCT_ALLOC_LWO;
inherit LIB_BIGSTRUCT_MAP_ITERATOR;

static void create(int clone)
{
	if (clone) {
		::create();
	}
}

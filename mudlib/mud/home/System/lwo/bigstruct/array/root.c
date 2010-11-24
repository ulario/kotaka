#include <kotaka/paths.h>
#include <kotaka/bigstruct.h>

inherit SECOND_AUTO;
inherit LIB_BIGSTRUCT_ALLOC_LWO;
inherit LIB_BIGSTRUCT_ARRAY_ROOT;

static void create(int clone)
{
	if (clone) {
		::create();
	}
}

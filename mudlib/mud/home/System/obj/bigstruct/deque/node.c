#include <kotaka/paths.h>
#include <kotaka/bigstruct.h>

inherit SECOND_AUTO;
inherit LIB_BIGSTRUCT_DEQUE_NODE;

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

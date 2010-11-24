#include <kernel/user.h>

#include <kotaka/paths.h>

inherit LIB_FILTER;

static void create(int clone)
{
	if (clone) {
		::create();
	}
}

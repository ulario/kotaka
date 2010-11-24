#include <kotaka/privilege.h>
#include <kotaka/paths.h>

inherit base LIB_OBJECT;

static void create(int clone)
{
	if (clone) {
		base::create();
	}
}

static void destruct(int clone)
{
	if (clone) {
		base::destruct();
	}
}

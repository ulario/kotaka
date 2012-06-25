#include <kotaka/paths.h>
#include <kotaka/privilege.h>

inherit base "~/lib/object";

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

/** Sits at the top of the object hierarchy */

#include <kotaka/privilege.h>
#include <kotaka/paths.h>

inherit base "~/lib/object";

static void create()
{
	::create();

	::set_id_base("ROOT");
}

void move(mixed args ...)
{
	error("Operation not permitted");
}

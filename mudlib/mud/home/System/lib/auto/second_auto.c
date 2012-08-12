#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>

#include <trace.h>
#include <type.h>
#include <status.h>

inherit "call_guard";
inherit "callout_guard";
inherit "touch";
inherit "object";

object canary;

/**********/
/* status */
/**********/

void set_canary(object new_canary)
{
	ACCESS_CHECK(SYSTEM());

	canary = new_canary;
}

object query_canary()
{
	ACCESS_CHECK(SYSTEM());

	return canary;
}

nomask void _F_dummy()
{
}

nomask mixed _F_status(mixed args ...)
{
	ACCESS_CHECK(SYSTEM());

	return status(this_object(), args ...);
}

#if 0
nomask void save_object(string filename)
{
	error("save_object is disabled");
}

nomask void restore_object(string filename)
{
	error("restore_object is disabled");
}
#endif

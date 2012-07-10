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

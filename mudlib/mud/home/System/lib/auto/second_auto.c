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

object objd_canary;

/**********/
/* status */
/**********/

nomask void _F_dummy()
{
}

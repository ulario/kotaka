/** Common bootstrap

Loads all the non-critical stuff that isn't in System
*/
#include <config.h>
#include <type.h>

#include <kernel/kernel.h>
#include <kernel/access.h>

#include <kotaka/paths.h>
#include <kotaka/channel.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>

inherit LIB_INITD;
inherit UTILITY_COMPILE;

static void create()
{
	load_dir("obj", 1);
	load_dir("sys", 1);
}

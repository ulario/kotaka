#include <kotaka/paths.h>

inherit LIB_INITD;
inherit UTILITY_COMPILE;

static void create()
{
	load_dir("obj", 1);
	load_dir("sys", 1);
}

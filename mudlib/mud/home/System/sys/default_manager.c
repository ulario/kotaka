#include <kotaka/paths.h>

inherit SECOND_AUTO;
inherit LIB_MANAGER;

string default_message;

static void create()
{
	default_message = "Internal error: no connection manager\n";
}

/* hook defaults for LIB_MANAGER */
string query_banner(object dummy)
{
	return default_message;
}

int query_timeout(object dummy)
{
	return -1;
}

object select(string dummy)
{
	return find_object(SYSTEM_DEFAULT_USER);
}

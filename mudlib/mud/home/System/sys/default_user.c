#include <kernel/user.h>
#include <kotaka/paths.h>

inherit SECOND_AUTO;
inherit LIB_USER;

/* hook defaults for LIB_USER */
int login(string str)
{
	previous_object()->message("Internal error: connection manager fault\n");
	return MODE_DISCONNECT;
}

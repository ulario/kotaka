#include <kernel/kernel.h>
#include <kernel/user.h>

#include <kotaka/paths.h>

inherit SECOND_AUTO;
inherit user LIB_USER;

static void create()
{
	user::create();
}

static void set_mode(int new_mode)
{
	if (this_object() && query_conn()) {
		query_conn()->set_mode(new_mode);
	}
}

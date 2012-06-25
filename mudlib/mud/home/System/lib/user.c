#include <kernel/kernel.h>
#include <kernel/user.h>

#include <kotaka/paths.h>

inherit SECOND_AUTO;
inherit user LIB_USER;

static void create()
{
	user::create();
}

static void redirect(object LIB_USER user, string str)
{
	/* intercept redirects in case we have a connection manager */
	/* bypassing the select() call in portd */
	object conn;
	object base_conn;

	if (!user) {
		error("Bad redirect");
	}

	conn = query_conn();
	base_conn = conn;

	while (base_conn && base_conn <- LIB_USER) {
		base_conn = base_conn->query_conn();
	}

	if (!conn) {
		error("Bad redirect");
	}

	/* will pile on mandated filters and then pass the connection off */
	/* to the requested user */
	::redirect(SYSTEM_USERD->intercept(conn, user), str);
}

static void set_mode(int new_mode)
{
	if (this_object() && query_conn()) {
		query_conn()->set_mode(new_mode);
	}
}

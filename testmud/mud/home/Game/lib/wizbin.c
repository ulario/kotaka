#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

static void wiztool_call(string function, varargs string args ...)
{
	object proxy;
	string name;
	string *messages;
	int sz, i;

	name = query_user()->query_name();
	proxy = PROXYD->get_proxy(name);

	call_other(proxy, function, args ...);

	messages = proxy->query_messages();

	sz = sizeof(messages);

	for (i = 0; i < sz; i++) {
		query_ustate()->send_out(messages[i]);
	}
}

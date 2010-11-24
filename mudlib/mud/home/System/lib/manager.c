#include <kotaka/privilege.h>
#include <kotaka/paths.h>

inherit SECOND_AUTO;

object select(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == PORTD);

	return nil;
}

string query_banner(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == PORTD);

	return "Welcome to Kotaka\n";
}

string query_blocked_message(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == PORTD);

	return "Sorry, but connections are currently blocked.\n";
}

string query_overload_message(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == PORTD);

	return "Sorry, but your connection would exceed system limitations.\n";
}

int query_timeout(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == PORTD);

	return 60;
}

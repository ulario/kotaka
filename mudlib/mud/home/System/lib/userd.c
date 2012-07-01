#include <kotaka/privilege.h>
#include <kotaka/paths.h>

inherit SECOND_AUTO;

string query_banner(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return "Welcome to Kotaka\n";
}

string query_blocked_banner(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return "The mud is undergoing maintenance.\n";
}

string query_overload_banner(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return "The mud is too full to accept any more connections.\n";
}

int query_timeout(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return 60;
}

object select(string str)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return find_object(SYSTEM_USERD);
}

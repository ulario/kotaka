#include <kotaka/paths.h>

inherit "utility";

static object query_ustate()
{
	return TLSD->query_tls_value("Game", "ustate");
}

static object query_user()
{
	return query_ustate()->query_user();
}

static void send_in(string str)
{
	query_ustate()->send_in(str);
}

static void send_out(string str)
{
	query_ustate()->send_out(str);
}

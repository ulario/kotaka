#include <kernel/user.h>

#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;
inherit LIB_FILTER;

static void create(int clone)
{
	if (clone) {
		::create();
	}
}

static int limited_login(string str)
{
	connection(previous_object(1));
	::open(nil);

	return ::receive_message(str);
}

static void limited_logout(int quit)
{
	::logout(quit);
}

static int limited_receive_message(string str)
{
	return ::receive_message(str);
}

static int limited_message_done()
{
	return ::message_done();
}

#ifndef SYS_NETWORKING
static void limited_open_datagram()
{
	::open_datagram();
}
#endif

static void limited_receive_datagram(string packet)
{
	::receive_datagram(packet);
}

int login(string str)
{
	ACCESS_CHECK(previous_object()->query_user() == this_object());
	ACCESS_CHECK(previous_program() == LIB_CONN);

	return call_limited("limited_login", str);
}

void logout(int quit)
{
	ACCESS_CHECK(previous_object()->query_user() == this_object());
	ACCESS_CHECK(previous_program() == LIB_CONN);

	call_limited("limited_logout", quit);
}

int receive_message(string str)
{
	ACCESS_CHECK(previous_object()->query_user() == this_object());
	ACCESS_CHECK(previous_program() == LIB_CONN);

	return call_limited("limited_receive_message", str);
}

int message_done()
{
	ACCESS_CHECK(previous_object()->query_user() == this_object());
	ACCESS_CHECK(previous_program() == LIB_CONN);

	return call_limited("limited_message_done");
}

#ifndef SYS_NETWORKING
void open_datagram()
{
	ACCESS_CHECK(previous_object()->query_user() == this_object());
	ACCESS_CHECK(previous_program() == LIB_CONN);

	call_limited("limited_open_datagram");
}
#endif

void receive_datagram(string packet)
{
	ACCESS_CHECK(previous_object()->query_user() == this_object());
	ACCESS_CHECK(previous_program() == LIB_CONN);

	call_limited("limited_receive_datagram", packet);
}

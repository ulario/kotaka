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

atomic int atomic_login(string str, object conn)
{
	int newmode;

	ACCESS_CHECK(previous_program() == LIB_CONN
		|| calling_object() == this_object());

	connection(conn);
	::open(nil);

	newmode = ::receive_message(str);

	if (!this_object()) {
		return MODE_DISCONNECT;
	}

	return newmode;
}

atomic static void atomic_logout(int quit)
{
	::logout(quit);
}

atomic static int atomic_receive_message(string str)
{
	return ::receive_message(str);
}

atomic static int atomic_message_done()
{
	return ::message_done();
}

atomic static void atomic_open_datagram()
{
	::open_datagram();
}

atomic static void atomic_receive_datagram(string packet)
{
	::receive_datagram(packet);
}

int login(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	return atomic_login(str, previous_object());
}

void logout(int quit)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	atomic_logout(quit);
}

int receive_message(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	return atomic_receive_message(str);
}

int message_done()
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	return atomic_message_done();
}

void open_datagram()
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	atomic_open_datagram();
}

void receive_datagram(string packet)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	atomic_receive_datagram(packet);
}

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

private atomic int atomic_login(string str)
{
	return ::login(str);
}

private atomic void atomic_logout(int quit)
{
	::logout(quit);
}

private atomic int atomic_receive_message(string str)
{
	return ::receive_message(str);
}

private atomic int atomic_message_done()
{
	return ::message_done();
}

# ifndef SYS_NETWORKING
private atomic void atomic_open_datagram()
{
	::open_datagram();
}
# endif

private atomic void atomic_receive_datagram(string packet)
{
	::receive_datagram(packet);
}

int login(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	return atomic_login(str);
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

# ifndef SYS_NETWORKING
void open_datagram()
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	atomic_open_datagram();
}
# endif

void receive_datagram(string packet)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	atomic_receive_datagram(packet);
}

#include <kernel/user.h>
#include <kotaka/log.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/text.h>
#include <kotaka/privilege.h>
#include <kotaka/telnet.h>

#define DELAY 0.005

inherit LIB_FILTER;

int dead;
string buffer;

static void create(int clone)
{
	if (clone) {
		::create();
	}
}

static void drain()
{
	if (strlen(buffer)) {
		call_out_unique("drain", DELAY);

		::message(buffer[0 .. 0]);

		buffer = buffer[1 ..];
	} else {
		if (dead) {
			::disconnect();
		} else {
			::message_done();
		}
	}
}

int login(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	buffer = "";

	return ::login(str);
}

int message(string str)
{
	ACCESS_CHECK(previous_program() == LIB_USER || previous_object() == query_user());

	buffer += str;

	call_out_unique("drain", DELAY);

	return 1;
}

int message_done()
{
	return MODE_NOCHANGE;
}

void disconnect()
{
	if (strlen(buffer)) {
		dead = 1;
		close(nil, 1);
		set_mode(MODE_BLOCK);
	} else {
		::disconnect();
	}
}

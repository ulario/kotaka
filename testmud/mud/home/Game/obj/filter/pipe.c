#include <kernel/user.h>

#include <kotaka/paths.h>

inherit LIB_FILTER;

static void create(int clone)
{
	if (clone) {
		::create();
	}
}

int login(string str)
{
	::login(nil);
	return ::receive_message(str);
}

#include <kernel/kernel.h>
#include <kotaka/privilege.h>

mapping touches;

static void call_touch(object obj)
{
	string cpath;
	string opath;

	cpath = object_name(previous_object());
	opath = object_name(obj);

	ACCESS_CHECK(DRIVER->creator(cpath) == DRIVER->creator(opath));

	::call_touch(obj);
}

static int touch(string function)
{
	if (sscanf(object_name(this_object()), USR_DIR + "/System/%*s")) {
		return 1;
	}
}

nomask int _F_call_touch(string function)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	return touch(function);
}

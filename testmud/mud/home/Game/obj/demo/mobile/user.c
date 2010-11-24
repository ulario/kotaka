#include <kotaka/privilege.h>

inherit "~/lib/mobile";

object ustate;
object body;

static void create(int clone)
{
	if (clone) {
		::create();
	}
}

int linkdead()
{
	return !!ustate;
}

void set_play_ustate(object new_ustate)
{
	ustate = new_ustate;
}

void set_body(object new_body)
{
	body = new_body;
}

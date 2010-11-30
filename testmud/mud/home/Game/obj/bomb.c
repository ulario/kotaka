#include <status.h>

#include <kotaka/paths.h>
#include <kotaka/log.h>

static void create(int clone)
{
	if (clone) {
		call_out("check", 0);
	}
}

static void check()
{
	object testd;

	if (!(testd = find_object("~/sys/testd")) ) {
		destruct_object(this_object());
		return;
	}
	
	call_out("check", SUBD->rnd() * (float)status(ST_NOBJECTS) / 1000.0);
}

static void self_destruct()
{
	destruct_object(this_object());
}

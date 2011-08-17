#include <status.h>

#include <kotaka/paths.h>
#include <kotaka/log.h>

#include "~/test.h"

static void create(int clone)
{
	if (clone) {
		call_out("check", TEST_CO_INTERVAL);
	}
}

static void check()
{
	object testd;

	if (!(testd = find_object("~/sys/testd")) ) {
		destruct_object(this_object());
		return;
	}

	call_out("check", SUBD->rnd() * (float)TEST_CO_INTERVAL);
}

static void self_destruct()
{
	destruct_object(this_object());
}

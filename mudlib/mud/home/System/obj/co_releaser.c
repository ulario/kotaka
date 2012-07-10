#include <kotaka/paths.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;

int charges;

static void create(int clone)
{
	if (clone) {
		call_out("release", 0);
	}
}

void charge(int new_charges)
{
	ACCESS_CHECK(SYSTEM());

	while (new_charges-- > 0 && free_callouts() > 60) {
		call_out("release", 0);
		charges++;
	}
}

static void release()
{
	if (charges--) {
		CALLOUTD->do_release();
	} else {
		destruct_object(this_object());
	}
}

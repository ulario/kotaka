#include <kotaka/paths.h>
#include <kotaka/log.h>
#include "~/test.h"

#include <status.h>

static void create()
{
}

void ignite(int count)
{
	call_out("bomb", 0, count);
}

static void suspend()
{
	LOGD->post_message("test", LOG_INFO, "Suspending callouts for " + (TEST_CO_INTERVAL + 10) + " seconds");

	CALLOUTD->hold_callouts((float)(TEST_CO_INTERVAL + 10));

	LOGD->post_message("test", LOG_INFO, "Self destructing");
	destruct_object("~/obj/bomb");
	destruct_object(this_object());
}

static void bomb(int quota)
{
	int max;

	max = (int)sqrt((float)quota);

	if (quota % max != 0) {
		max = quota % max;
	}

	if (max > quota) {
		max = quota;
	}

	for (; quota > 0 && max > 0; quota--, max--) {
		clone_object("~/obj/bomb");
	}

	LOGD->post_message("test", LOG_INFO, quota + " bombs left to clone.");

	if (quota > 0) {
		call_out("bomb", 0, quota);
	} else {
		suspend();
	}
}

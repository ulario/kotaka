#include <kotaka/paths.h>
#include <kotaka/log.h>

#include <status.h>

static void create()
{
}

void ignite()
{
	call_out("bomb", 0, 1000000);
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
	}
}

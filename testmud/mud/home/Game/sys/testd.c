#include <kotaka/paths.h>
#include <kotaka/log.h>

static void create()
{
	call_out("bomb", 0, 100000);
}

static void bomb(int quota)
{
	int max;
	
	max = 100000;
	
	if (max > quota) {
		max = quota;
	}
	
	for (; quota > 0 && max > 0; quota--, max--) {
		clone_object("~/obj/bomb");

		if (quota % 1000 == 0) {
			LOGD->post_message("test", LOG_INFO, quota + " bombs left to clone.");
		}
	}
	
	if (quota > 0) {
		call_out("bomb", 0, quota);
	}
}

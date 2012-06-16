#include <status.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;

void extinguish(string path)
{
	ACCESS_CHECK(KADMIN());
	call_out("purge", 0, path, status(ST_OTABSIZE));
}

static void purge(string path, int quota)
{
	int max;

	max = 200;

	if (max > quota) {
		max = quota;
	}

	for (; quota > 0 && max > 0; quota--, max--) {
		object obj;

		obj = find_object(path + "#" + quota);

		if (obj) {
			destruct_object(obj);
		}
	}

	LOGD->post_message("test", LOG_INFO, quota + " objects to check.");

	if (quota > 0) {
		call_out("purge", 0, path, quota);
	}
}

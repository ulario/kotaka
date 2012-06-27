#include <status.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>

inherit SECOND_AUTO;

void extinguish(string path)
{
	if (!KADMIN()) {
		string opath;

		opath = object_name(previous_object());

		ACCESS_CHECK(DRIVER->creator(opath));
		ACCESS_CHECK(DRIVER->creator(opath) == DRIVER->creator(path));
	}

	call_out("purge", 0, path, status(ST_OTABSIZE));
}

static void purge(string path, int quota)
{
	int limit;

	limit = 128;

	if (quota % limit != 0) {
		limit = quota % limit;
	}

	for (; quota > 0 && limit > 0; quota--, limit--) {
		object obj;

		if (obj = find_object(path + "#" + quota)) {
			destruct_object(obj);
		}
	}

	LOGD->post_message("test", LOG_INFO, quota + " objects to check.");

	if (quota > 0) {
		call_out("purge", 0, path, quota);
	}
}

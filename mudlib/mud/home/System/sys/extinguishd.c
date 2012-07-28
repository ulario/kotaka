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

static void purge(string path, int index)
{
	int limit;
	int quota;

	quota = 256;

	if (index >= status(ST_OTABSIZE)) {
		index = status(ST_OTABSIZE) - 1;
	}

	if (index % quota > 0) {
		quota = index % quota;
	}

	for (; quota > 0 && index >= 0; quota--, index--) {
		object obj;

		if (obj = find_object(path + "#" + index)) {
			destruct_object(obj);
		}
	}

	if (index >= 0) {
		LOGD->post_message("extinguish", LOG_DEBUG, "Continuing purge at " + index);
		call_out("purge", 0, path, index);
	}
}

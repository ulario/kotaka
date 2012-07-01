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

	call_out("purge", 0, path, 0);
}

static void purge(string path, int index)
{
	int limit;
	int quota;

	quota = 100;
	quota -= index % quota;
	limit = status(ST_OTABSIZE);

	for (; quota > 0 && index < limit; quota--, index++) {
		object obj;

		if (obj = find_object(path + "#" + index)) {
			destruct_object(obj);
		}
	}

	if (index < limit) {
		call_out("purge", 0, path, index);
	}
}

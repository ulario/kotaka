#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/assert.h>
#include <kotaka/privilege.h>
#include "~/test.h"

#include <status.h>

static void create()
{
}

/* bomb tests */

void ignite(int count)
{
	call_out("bomb", 0, count);
}

void touchall()
{
	int i;

	LOGD->post_message("test", LOG_INFO, "Beginning touch");

	call_out("do_touch", 0, status(ST_OTABSIZE) - 1);
}

static void do_touch(int quota)
{
	int limit;

	limit = 1024;

	if (quota % limit != 0) {
		limit = quota % limit;
	}

	for (; quota >= 0 && limit > 0; quota--, limit--) {
		object bomb;

		if (bomb = find_object("~/obj/bomb" + quota)) {
			call_touch(bomb);
		}
	}

	LOGD->post_message("test", LOG_INFO, quota + " objects left to check for touches.");

	if (quota > 0) {
		call_out("do_touch", 0, quota);
	}
}

static void bomb(int quota)
{
	int limit;

	limit = 100;
	limit += quota % limit;

	for (; quota > 0 && limit > 0; quota--, limit--) {
		clone_object("~/obj/bomb");
	}

	LOGD->post_message("test", LOG_INFO, quota + " bombs left to clone.");

	if (quota > 0) {
		call_out("bomb", 0, quota);
	}
}

void test()
{
	ACCESS_CHECK(GAME());
}

#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/assert.h>
#include <kotaka/privilege.h>
#include "~/test.h"

#include <status.h>

static void create()
{
}

void test()
{
	ACCESS_CHECK(GAME());
}

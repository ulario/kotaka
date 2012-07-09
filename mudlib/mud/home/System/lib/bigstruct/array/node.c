#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

inherit SECOND_AUTO;
inherit "../base/node";

mixed *array;
int level;

static void create()
{
	::create();

	level = -1;
	array = allocate(256);
}

void set_level(int new_level)
{
	check_caller();

	ASSERT(level == -1);

	level = new_level;
}

int get_level()
{
	check_caller();

	return level;
}

mixed *get_array()
{
	check_caller();

	return array;
}

#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

inherit SECOND_AUTO;

private object root;

static void create()
{
	root = previous_object();
}

static void check_caller()
{
	ACCESS_CHECK(previous_object() == root || sscanf(object_name(previous_object()),
		USR_DIR + "/System/%*s/bigstruct/"));
}

nomask object query_root()
{
	return root;
}

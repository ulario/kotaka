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

static void check_caller(int access)
{
	object pobj;
	
	pobj = previous_object();
	
	if (!sscanf(object_name(pobj),
		USR_DIR + "/System/%*s/bigstruct/%*s") == 2
	) {
		ACCESS_CHECK(root->access_of(pobj) >= access);
	}
}

object query_root()
{
	return root;
}

void increment()
{
	error("Must override");
}

void decrement()
{
	error("Must override");
}

mixed get_data()
{
	error("Must override");
}

void set_data(mixed data)
{
	error("Must override");
}

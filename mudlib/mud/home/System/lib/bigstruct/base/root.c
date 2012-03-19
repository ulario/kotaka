#include <kernel/access.h>

#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

inherit SECOND_AUTO;

private object struct_owner;

private mapping grants;
private int global_access;

static object new_node();
static void discard_node(object node);

static void create()
{
	object pobj;
	object struct_owner;
	
	grants = ([ ]);
	pobj = previous_object();

	if (pobj <- "root") {
		grants = pobj->query_grants()[..];
	} else {
		grants[pobj] = FULL_ACCESS;
	}
}

int access_of(object obj)
{
	int highest;

	ACCESS_CHECK(SYSTEM());

	if (grants[obj]) {
		highest = grants[obj];
	}

	if (highest > global_access) {
		return highest;
	} else {
		return global_access;
	}
}

static void check_caller(int access)
{
	object pobj;

	pobj = previous_object();

	if (!sscanf(object_name(pobj),
		USR_DIR + "/System/%*s/bigstruct/"))
	{
		ACCESS_CHECK(access_of(pobj) >= access);
	}
}

mapping query_grants()
{
	check_caller(READ_ACCESS);

	return grants[..];
}

int query_global_access()
{
	check_caller(READ_ACCESS);

	return global_access;
}

void grant_access(object obj, int access)
{
	check_caller(FULL_ACCESS);

	grants[obj] = access;
}

void grant_global_access(int access)
{
	check_caller(FULL_ACCESS);

	global_access = access;
}

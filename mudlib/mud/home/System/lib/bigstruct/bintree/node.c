#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <status.h>

inherit SECOND_AUTO;
inherit "../base/node";

private object parent;
private object left;
private object right;

/* get */

object get_parent()
{
	check_caller();
	return parent;
}

object get_left()
{
	check_caller();
	return left;
}

object get_right()
{
	check_caller();
	return right;
}

/* set */

void set_parent(object new)
{
	check_caller();
	parent = new;
}

void set_left(object new)
{
	check_caller();
	left = new;
}

void set_right(object new)
{
	check_caller();
	right = new;
}

#include <kotaka/paths.h>

inherit SECOND_AUTO;

static object new_node()
{
	return clone_object("node");
}

static void discard_node(object node)
{
	destruct_object(node);
}

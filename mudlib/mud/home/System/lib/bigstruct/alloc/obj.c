#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/assert.h>

static object new_node()
{
	return clone_object("node");
}

static void discard_node(object node)
{
	destruct_object(node);
}

#include <kotaka/paths.h>

inherit LIB_OBJECT;

inherit "position";

/*****************/
/* General stuff */
/*****************/

static void create()
{
	string name;
	string *parts;

	name = object_name(this_object());

	sscanf(name, "%s#%*d", name);

	parts = explode(name, "/");

	set_id_base(parts[sizeof(parts) - 1]);
}

static void gobj_destruct()
{
	int sz;
	int index;
	object env;
	object *children;

	children = query_inventory();
	env = query_environment();
	sz = sizeof(children);

	for (index = 0; index < sz; index++) {
		children[index]->move(env);
	}
}

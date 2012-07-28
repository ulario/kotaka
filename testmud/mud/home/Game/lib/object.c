#include <kotaka/paths.h>

inherit base LIB_OBJECT;

inherit "position";
inherit bulk "bulk";

int destructing;

/*****************/
/* General stuff */
/*****************/

static void create()
{
	string name;
	string *parts;
	int sz;

	base::create();
	bulk::create();

	name = object_name(this_object());

	sscanf(name, "%s#%*d", name);

	parts = explode(name, "/");
	sz = sizeof(parts);

	set_id_base(parts[sz - 1]);
}

int forbid_insert(object obj)
{
	return destructing;
}

static nomask void game_object_destruct()
{
	int sz;
	int index;
	object env;
	object *children;

	destructing = 1;

	children = query_inventory();
	env = query_environment();
	sz = sizeof(children);

	for (index = 0; index < sz; index++) {
		children[index]->move(env);
	}
}

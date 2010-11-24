#include <kotaka/paths.h>

inherit LIB_OBJECT;

/*****************/
/* General stuff */
/*****************/

static void destruct()
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

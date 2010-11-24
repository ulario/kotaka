#include <kotaka/checkarg.h>
#include <kotaka/privilege.h>
#include <kotaka/assert.h>

#include <kotaka/paths.h>

inherit LIB_DRIVER;

static void create()
{
}

object make_lwo(string path)
{
	ACCESS_CHECK(KOTAKA() || GAME());
	
	return new_object(path);
}

object make_clone(string path)
{
	ACCESS_CHECK(KOTAKA() || GAME());
	
	return clone_object(path);
}

object create_object(varargs object LIB_OBJECT *parents)
{
	object new;

	new = ::clone_object("~/obj/object");

	if (parents) {
		new->set_archetypes( parents );
	}
	
	new->created();
	
	return new;
}

atomic void destroy_object(object LIB_OBJECT turkey,
	varargs int recursive)
{
	if (!turkey) {
		BADARG(1, "destroy_object");
	}

	if (sizeof(turkey->query_inventory())) {
		if (recursive) {
			object *inv;
			object subturkey;
			int index;

			inv = turkey->query_inventory();

			for (index = 0; index < sizeof(inv); index++) {
				subturkey = inv[index];

				destroy_object(subturkey, 1);
			}
		} else {
			error("Object not empty");
		}
	}

	destruct_object(turkey);
}

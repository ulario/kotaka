#include <kotaka/paths.h>
#include <game/paths.h>

/* overrides LIB_USTATE to provide auditing */

inherit LIB_USTATE;

static void terminate_account_state()
{
	object parent;

	parent = query_parent();

	if (parent <- "~/obj/ustate/start") {
		parent->swap_state(clone_object("shell"));
	} else {
		pop_state();
	}
}

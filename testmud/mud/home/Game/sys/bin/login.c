#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	query_ustate()->push_state(clone_object("~/obj/ustate/login"));
}

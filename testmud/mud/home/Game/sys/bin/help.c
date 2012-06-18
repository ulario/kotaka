#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	object pager;

	pager = clone_object("~Kotaka/obj/ustate/page");

	pager->set_text(read_file("~/data/help/player/index.hlp"));

	query_user()->push_state(pager);
}

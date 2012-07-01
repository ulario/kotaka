/** Grants proxies

A proxy is used to grant user level permissions to objects, or to allow
one subsystem to take advantage of grants made by other subsystems.

*/
#include <kernel/kernel.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/checkarg.h>

inherit SECOND_AUTO;

mapping wiztools;

static void create()
{
	wiztools = ([ ]);
}

object get_wiztool()
{
	string name;
	object wiztool;

	ACCESS_CHECK(PRIVILEGED());

	name = this_user()->query_name();

	if (!name) {
		error("Only users can have wiztools");
	}

	wiztool = wiztools[name];

	if (!wiztool) {
		wiztool = wiztools[name] = clone_object("~/obj/wiztool", name);
		wiztool->set_user(previous_object());
	}

	return wiztools[name];
}

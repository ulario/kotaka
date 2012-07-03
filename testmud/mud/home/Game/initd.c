#include <config.h>

#include <kernel/kernel.h>
#include <kernel/access.h>

#include <kotaka/privilege.h>
#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/property.h>

#include <game/paths.h>

#include <status.h>
#include <type.h>

/**************/
/* Directives */
/**************/

inherit LIB_INITD;
inherit UTILITY_COMPILE;

/*************/
/* Variables */
/*************/

object root;
int save_pending;
int wsave_pending;

/****************/
/* Declarations */
/****************/

string bits(string input);
static void do_test();

/****************/
/* Constructors */
/****************/

static void create()
{
	object proxy;
	string world;
	string dump;
	int index;
	object clone;
	object shape;
	string *bad_bins;
	string *bins;
	string test;

	load_dir("lwo", 1);
	load_dir("obj", 1);
	load_dir("open", 1);
	load_dir("sys", 1);

	PROXYD->get_proxy("Game")->set_global_access("~", 1);
	PROPERTYD->add_property("facing", T_INT, PROP_SIMPLE);
	PROPERTYD->add_property("holding", T_MAPPING, PROP_SIMPLE);

	root = find_object("sys/root");

	"sys/testd"->test();
}

void reboot()
{
}

/****************/
/* System hooks */
/****************/

string query_destructor(string path)
{
	switch(path) {
	case USR_DIR + "/Game/lib/object":
		return "game_object_destruct";
	}
}

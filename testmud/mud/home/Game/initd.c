#include <config.h>

#include <kernel/kernel.h>
#include <kernel/access.h>

#include <kotaka/privilege.h>
#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>

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

	root = find_object("sys/root");
}

void reboot()
{
}

string bits(string input)
{
	string buf;
	int a, b;

	buf = "";

	for (a = 0; a < strlen(input); a++) {
		int byte;
		
		byte = input[a];
		
		for (b = 0; b < 8; b++) {
			buf += ((byte >> b) & 1) ? "#" : "-";
		}
	}
	
	return buf;
}

/****************/
/* System hooks */
/****************/

string query_destructor(string path)
{
	switch(path) {
	case USR_DIR + "/Game/lib/time":
		return "time_destruct";
	case USR_DIR + "/Game/lib/object":
		return "gobj_destruct";
	default:
		LOGD->post_message("game", LOG_INFO, "No destructor registered for " + path);
	}
}

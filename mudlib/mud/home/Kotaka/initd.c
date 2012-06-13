/** Common bootstrap

Loads all the non-critical stuff that isn't in System
*/
#include <config.h>
#include <type.h>

#include <kernel/kernel.h>
#include <kernel/access.h>

#include <kotaka/paths.h>
#include <kotaka/channel.h>
#include <kotaka/log.h>

inherit LIB_INITD;
inherit UTILITY_COMPILE;

#define RED 1
#define GREEN 2
#define BLUE 4
#define BRIGHT 8

string *bad_bins;

private void setup_channels()
{
	CHANNELD->set_channel_config("debug", "post_access", "system");
	CHANNELD->set_channel_config("debug", "channel_color", GREEN);


	CHANNELD->set_channel_config("error", "join_access", "wizard");
	CHANNELD->set_channel_config("error", "post_access", "system");

	CHANNELD->set_channel_config("error", "channel_color", RED);
	CHANNELD->set_channel_config("error", "prefix_color", nil);
	CHANNELD->set_channel_config("error", "text_color", nil);


	CHANNELD->set_channel_config("warning", "join_access", "wizard");
	CHANNELD->set_channel_config("warning", "post_access", "system");

	CHANNELD->set_channel_config("warning", "channel_color", RED | GREEN);
	CHANNELD->set_channel_config("warning", "prefix_color", nil);
	CHANNELD->set_channel_config("warning", "text_color", nil);


	CHANNELD->set_channel_config("trace", "join_access", "wizard");
	CHANNELD->set_channel_config("trace", "post_access", "system");

	CHANNELD->set_channel_config("trace", "channel_color", RED | BLUE);
	CHANNELD->set_channel_config("trace", "prefix_color", nil);
	CHANNELD->set_channel_config("trace", "text_color", nil);


	CHANNELD->set_channel_config("compile", "join_access", "wizard");
	CHANNELD->set_channel_config("compile", "post_access", "system");

	CHANNELD->set_channel_config("compile", "channel_color", BLUE | GREEN);
	CHANNELD->set_channel_config("compile", "prefix_color", nil);
	CHANNELD->set_channel_config("compile", "text_color", nil);


	CHANNELD->add_channel("admin");
	CHANNELD->set_channel_config("admin", "join_access", "admin");

	CHANNELD->set_channel_config("admin", "channel_color", RED | BRIGHT);
	CHANNELD->set_channel_config("admin", "prefix_color", RED);
	CHANNELD->set_channel_config("admin", "text_color", nil);


	CHANNELD->add_channel("wizard");
	CHANNELD->set_channel_config("wizard", "join_access", "wizard");

	CHANNELD->set_channel_config("wizard", "channel_color", RED | GREEN | BLUE | BRIGHT);
	CHANNELD->set_channel_config("wizard", "prefix_color", BLUE | GREEN | BRIGHT);
	CHANNELD->set_channel_config("wizard", "text_color", BLUE | GREEN);


	CHANNELD->add_channel("chat");
	CHANNELD->set_channel_config("chat", "channel_color", GREEN | BRIGHT);
	CHANNELD->set_channel_config("chat", "prefix_color", BLUE | GREEN | BRIGHT);
	CHANNELD->set_channel_config("chat", "text_color", nil);


	CHANNELD->set_channel_config("connection", "post_access", "system");
	CHANNELD->set_channel_config("connection", "flags", CHFLAG_ANNOUNCE | CHFLAG_GLOBAL | CHFLAG_EXCLSENDER);

	CHANNELD->set_channel_config("connection", "channel_color", nil);
	CHANNELD->set_channel_config("connection", "prefix_color", nil);
	CHANNELD->set_channel_config("connection", "text_color", RED | GREEN | BLUE | BRIGHT);


	CHANNELD->add_channel("announcement");
	CHANNELD->set_channel_config("announcement", "post_access", "admin");
	CHANNELD->set_channel_config("announcement", "flags", CHFLAG_ANNOUNCE | CHFLAG_GLOBAL);

	CHANNELD->set_channel_config("announcement", "channel_color", nil);
	CHANNELD->set_channel_config("announcement", "prefix_color", BLUE | GREEN | BRIGHT);
	CHANNELD->set_channel_config("announcement", "text_color", BLUE | GREEN | BRIGHT);


	CHANNELD->add_channel("bug", 1);
	CHANNELD->set_channel_config("bug", "join_access", "wizard");
	CHANNELD->set_channel_config("bug", "post_access", "system");

	CHANNELD->set_channel_config("bug", "channel_color", BRIGHT | BLUE | GREEN);
	CHANNELD->set_channel_config("bug", "prefix_color", BLUE | RED);
	CHANNELD->set_channel_config("bug", "text_color", nil);


	CHANNELD->add_channel("typo", 1);
	CHANNELD->set_channel_config("typo", "join_access", "wizard");
	CHANNELD->set_channel_config("typo", "post_access", "system");

	CHANNELD->set_channel_config("typo", "channel_color", BRIGHT | BLUE | GREEN);
	CHANNELD->set_channel_config("typo", "prefix_color", BLUE | RED);
	CHANNELD->set_channel_config("typo", "text_color", nil);


	CHANNELD->add_channel("idea", 1);
	CHANNELD->set_channel_config("idea", "join_access", "wizard");
	CHANNELD->set_channel_config("idea", "post_access", "system");

	CHANNELD->set_channel_config("idea", "channel_color", BRIGHT | BLUE | GREEN);
	CHANNELD->set_channel_config("idea", "prefix_color", BLUE | RED);
	CHANNELD->set_channel_config("idea", "text_color", nil);


	CHANNELD->add_channel("nohelp", 1);
	CHANNELD->set_channel_config("nohelp", "join_access", "wizard");
	CHANNELD->set_channel_config("nohelp", "post_access", "system");

	CHANNELD->set_channel_config("nohelp", "channel_color", BRIGHT | BLUE | GREEN);
	CHANNELD->set_channel_config("nohelp", "prefix_color", BLUE | RED);
	CHANNELD->set_channel_config("nohelp", "text_color", nil);
}

/** Bootstrap */
static void create()
{
	int i;

	load_object(STRINGD);

	load_dir("lib", 1);
	load_dir("lwo", 1);
	load_dir("obj", 1);
	load_dir("open", 1);
	load_dir("sys", 1);

	setup_channels();

	TLSD->set_tls_access("Kotaka", "opwd", "Game", WRITE_ACCESS);
	TLSD->set_tls_access("Kotaka", "opwd", "System", READ_ACCESS);
	TLSD->set_tls_access("Kotaka", "env", "System", READ_ACCESS);

	"sys/testd"->test();
}

/** Controls access to Common inheritables */
int forbid_inherit(string from, string path, int priv)
{
}

/** Object constructor/destructor */
string query_constructor(string path)
{
	switch(path) {
	case LIB_OBJECT:
		return "kotaka_object_constructor";
	}
}

string query_destructor(string path)
{
	switch(path) {
	case LIB_OBJECT:
		return "kotaka_object_destructor";
	}
}

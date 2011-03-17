#include <kotaka/privilege.h>
#include <kernel/user.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/assert.h>

#include <game/paths.h>

inherit LIB_KOTAKA_USER;

object mobile;
int keepalive;

mapping aliases;
string *disabled;

static void create(int clone)
{
	if (clone) {
		::create();
		
		aliases = ([ ]);
		disabled = ({ });
	}
}

static void destruct(int clone)
{
	if (clone) {
		LOGD->post_message("game", LOG_INFO, "User destructing");
	}
}

void set_uid(int new_uid)
{
	ACCESS_CHECK(GAME());

	LOGD->post_message("game", LOG_INFO, "User has new UID: " + new_uid);
}

void set_mobile(object new_mobile)
{
	ACCESS_CHECK(GAME());

	mobile = new_mobile;
}

object query_mobile()
{
	return mobile;
}

void logout(int quit)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	::logout(quit);
}

int login(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);
	ASSERT(str == nil);

	connection(previous_object());

	set_mode(MODE_ECHO);

	set_root_state(clone_object("~/obj/ustate/bootstrap"));

	return MODE_NOCHANGE;
}

int receive_message(string str)
{
	int ret;
	
	ACCESS_CHECK(previous_program() == LIB_CONN);

	ret = ::receive_message(str);
	
	if (ret == MODE_DISCONNECT) {
		INITD->message("User is dying...");
	}

	set_mode(ret);

	return ret;
}

void message(string str)
{
	PERMISSION_CHECK(!(previous_object() <- LIB_USTATE));

	::message(str);
}

private string query_brief(object obj)
{
	string out;
	
	out = obj->query_property("bdesc");
	
	if (out) {
		return out;
	} else {
		return "an undescript item";
	}
}

void show_room()
{
	object body;
	object env;
	int index;
	int sz;
	
	string bdesc;
	string ldesc;
	
	mixed *sib;
	mapping exits;
	
	/* todo: do an XYZ scan of the surrounding area */
	/* Better yet, order the mobile to ask the body what it sees */

	ACCESS_CHECK(GAME());
	
	if (!mobile) {
		::feed_out("You don't exist.\n");
		return;
	}

	body = mobile->query_environment();
	
	if (!body) {
		::feed_out("Your bodyless mind cannot see anything.\n");
		return;
	}
	
	env = body->query_environment();
	
	if (!env) {
		::feed_out("You are in absolute void.\n");
		return;
	}
	
	/* todo: make exits formalized with distinct labels for each */
	/* exits are directions, not nouns */
	sib = env->query_inventory();
	
	sib -= ({ body });
	
	if (is_wizard()) {
		::feed_out("Location: " + env->query_path() + "\n\n");
	}
	
	bdesc = env->query_property("bdesc");
	ldesc = env->query_property("ldesc");
	
	if (!bdesc) {
		bdesc = "(an unlabeled room)";
	}
	
	if (!ldesc) {
		ldesc = "This room is utterly non-descript\n";
	}
	
	::feed_out(bdesc + "\n\n");
	::feed_out(ldesc + "\n");
	
	sz = sizeof(sib);
	
	if (sz) {
		for (index = 0; index < sz; index++) {
			if (sib[index]->query_property("is_invisible")) {
				sib[index] = nil;
			} else if (sib[index]->query_property("is_scenery")) {
				sib[index] = nil;
			} else {
				sib[index] = query_brief(sib[index]);
			}
		}
	}

	sib -= ({ nil });
	sz = sizeof(sib);

	if (sz) {
		::feed_out("You see:\n");
		for (index = 0; index < sz; index++) {
			::feed_out("- " + sib[index] + "\n");
		}
		::feed_out("\n");
	}

	exits = env->query_property("exits");

	if (map_sizeof(exits)) {
		::feed_out("Exits: " + implode(map_indices(exits), ", ") + "\n");
		sib -= map_values(exits);
	}
	
	mobile->set_property("last_environment", env);
}

void set_aliases(mapping new_aliases)
{
	ACCESS_CHECK(previous_object() == query_root());
	
	aliases = new_aliases;
}

void set_disabled_aliases(string *new_disabled)
{
	ACCESS_CHECK(previous_object() == query_root());

	disabled = new_disabled;
}

mapping query_aliases()
{
	return aliases[..];
}

string *query_disabled_aliases()
{
	return disabled[..];
}

void add_alias(string prefix, string output)
{
	ACCESS_CHECK(GAME());

	PERMISSION_CHECK(prefix != "aliases");
	PERMISSION_CHECK(prefix != "alias");
	PERMISSION_CHECK(prefix != "unalias");

	aliases[prefix] = output;
}

void del_alias(string prefix)
{
	ACCESS_CHECK(GAME());
	
	aliases[prefix] = nil;
}

string query_alias(string prefix)
{
	return aliases[prefix];
}

void quit()
{
	disconnect();
}

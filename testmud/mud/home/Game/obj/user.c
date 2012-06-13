#include <kotaka/privilege.h>
#include <kernel/user.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/assert.h>
#include <kernel/access.h>

#include <game/paths.h>

inherit LIB_KOTAKA_USER;

object mobile;
int keepalive;

string username;

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
	}
}

string query_username()
{
	return username;
}

int query_class()
{
	if (!username) {
		return 0;
	}

	if (!ACCOUNTD->query_is_registered(username)) {
		return 0;
	}

	if (KERNELD->access(username, "/", FULL_ACCESS)) {
		return 3;
	}

	if (sizeof( KERNELD->query_users() & ({ username }) )) {
		return 2;
	}

	return 1;
}

void set_username(string new_username)
{
	ACCESS_CHECK(GAME());

	username = new_username;
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

	set_root_state(clone_object("~/obj/ustate/start"));

	return MODE_NOCHANGE;
}

int receive_message(string str)
{
	int ret;
	object conn, conn2;
	string ip;

	ACCESS_CHECK(previous_program() == LIB_CONN);
	conn = previous_object();

	while (conn <- LIB_USER) {
		conn = conn->query_conn();
	}

	ip = query_ip_number(conn);

	write_file("~/log",
		(username ? username :
		(
			"(" + (ip ? ip : "nil") + ")"
		))
		+ ": " + str + "\n"
	);

	ret = ::receive_message(str);

	if (ret == MODE_DISCONNECT) {
		INITD->message("User is dying...");
	}

	set_mode(ret);

	return ret;
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

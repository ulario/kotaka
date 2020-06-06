#include <kernel/user.h>
#include <kotaka/paths/system.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>

inherit "/lib/string/sprint";
inherit LIB_USERD;
inherit LIB_SYSTEM_USER;

mapping conns;

/* private */

private object root_of(object conn)
{
	while (conn <- LIB_USER) {
		conn = conn->query_conn();
	}

	return conn;
}

/* creator */

static void create()
{
	conns = ([ ]);

	SYSTEM_USERD->set_binary_manager(3, this_object());

	call_out("heartbeat", 5);
}

static void heartbeat()
{
	object *list;
	int sz;

	call_out("heartbeat", 5);

	list = map_indices(conns);

	for (sz = sizeof(list); --sz >= 0; ) {
		LOGD->post_message("debug", LOG_DEBUG, "CommD: Sending heartbeat to " + conns[list[sz]]);
		list[sz]->message("Heartbeat\n");
	}
}

/* userd hooks */

string query_banner(object conn)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	conn = root_of(conn);

	return "Welcome to CommD, this session's random number is " + random(1000000000) + "\n";
}

int query_timeout(object conn)
{
	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	return 10;
}

object select(string str)
{
	object conn;

	ACCESS_CHECK(previous_program() == SYSTEM_USERD);

	/* 2: BINARY_CONN */
	/* 1: USERD */
	/* 0: SYSTEM_USERD */
	conn = previous_object(2);

	conn = root_of(conn);

	return this_object();
}

/* user hooks */

void connect_failed(int refused)
{
	ACCESS_CHECK(previous_program() == SYSTEM_CONN);

	LOGD->post_message("debug", LOG_DEBUG, "CommD: connect_failed, refused is " + refused);
}

int login(string str)
{
	object conn;
	object root;
	string ip;

	ACCESS_CHECK(previous_program() == LIB_CONN);

	conn = previous_object();
	root = conn;

	while (root <- LIB_USER) {
		root = root->query_conn();
	}

	ip = query_ip_number(root);

	conns[conn] = ip;

	conn->message("Hello " + ip + "\n");

	LOGD->post_message("debug", LOG_DEBUG, "CommD: login from " + ip + ", first line is " + str);

	return MODE_NOCHANGE;
}

void logout(int quit)
{
	object conn;

	ACCESS_CHECK(previous_program() == LIB_CONN);

	conn = root_of(previous_object());

	LOGD->post_message("debug", LOG_DEBUG, "CommD: logout from " + object_name(conn) + ", quit is " + quit);
}

int receive_message(string str)
{
	object conn;

	ACCESS_CHECK(previous_program() == LIB_CONN);

	conn = root_of(previous_object());

	LOGD->post_message("debug", LOG_DEBUG, "CommD: receive_message from " + conns[conn] + ", line is " + str);

	return MODE_NOCHANGE;
}

int message_done()
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	return MODE_NOCHANGE;
}

/* public */

void link(string target)
{
	ACCESS_CHECK(VERB());

	connect(target, 50010);
}

void unlink(string target)
{
	mixed *ind;
	int sz;

	ACCESS_CHECK(VERB());

	if (!conns[target]) {
		error("No such target linked");
	}

	ind = map_indices(conns);

	for (sz = sizeof(ind); --sz >= 0; ) {
		if (conns[ind[sz]] == target) {
			ind[sz]->disconnect();
		}
	}
}

mapping query_conns()
{
	return conns[..];
}

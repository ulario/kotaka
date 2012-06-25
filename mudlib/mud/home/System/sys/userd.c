/*

Port manager

Handles incoming connections.

*/
#include <kernel/kernel.h>
#include <kernel/user.h>

#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>

#include <status.h>

inherit SECOND_AUTO;

inherit userd API_USER;
inherit user LIB_USER;

/* Number of user slots to keep spare for the Klib emergency login port */
#define SPARE_USERS		5
#define MAX_CONN_DEPTH		10

#define CINFO_TYPE	0	/* binary, telnet */
#define CINFO_LPORT	1	/* logical port */
#define CINFO_PPORT	2	/* physical port */

/*************/
/* Variables */
/*************/

mapping telnet_managers;	/* managers assigned to logical telnet ports */
mapping binary_managers;	/* managers assigned to logical binary ports */
mapping fixed_managers;		/* managers assigned to physical ports */

mapping connections;		/* ([ connection : ({ type, lport, pport }) ]) */
mapping intercepts;		/* ([ connection : user ]) */

int enabled;		/* active or not */
int binary_port_count;	/* number of ports currently registered */
int telnet_port_count;	/* number of ports currently registered */
int blocked;		/* connection blocking is active */
mapping reblocked;	/* objects that were already manually blocked */
int stacking;		/* if we are currently building the connection stack */

/****************/
/* Declarations */
/****************/

/* internal */

static void create();
int login(string str);
private void register_with_klib_userd();
private void unregister_with_klib_userd();
private object get_manager(object conn);

/* external */

void reboot();
void clear_ports();

void set_binary_manager(int port, object LIB_USERD manager);
void set_telnet_manager(int port, object LIB_USERD manager);
void set_fixed_manager(int port, object LIB_USERD manager);

int free_users();
void block_connections();
void unblock_connections();
int query_connection_count();

/* initd hooks */

/***************/
/* Definitions */
/***************/

/* internal */

static void create()
{
	user::create();
	userd::create();

	binary_managers = ([ ]);
	telnet_managers = ([ ]);
	fixed_managers = ([ ]);

	connections = ([ ]);
	intercepts = ([ ]);

	reblocked = ([ ]);
}

private void analyze_connection(object LIB_CONN connection)
{
	string path;
	mixed *cinfo;
	int lport;

	if (connections[connection]) {
		error("Duplicate analysis of connection");
	}

	path = object_name(connection);
	sscanf(path, "%s#%*d", path);

	switch(path) {
	case TELNET_CONN:
		lport = connection->query_port();
		cinfo = ({ "telnet", lport, status()[ST_TELNETPORTS][lport] });
		break;

	case BINARY_CONN:
		lport = connection->query_port();
		cinfo = ({ "binary", lport, status()[ST_BINARYPORTS][lport] });
		break;

	default:
		error("Unrecognized connection type: " + path);
	}

	connections[connection] = cinfo;
}

private void register_with_klib_userd()
{
	mixed *status;
	int index;
	object this;
	this = this_object();

	status = status();

	telnet_port_count = sizeof(status[ST_TELNETPORTS]);
	binary_port_count = sizeof(status[ST_BINARYPORTS]);

	USERD->set_telnet_manager(0, this);

	for (index = 1; index < telnet_port_count; index++) {
		USERD->set_telnet_manager(index, this);
	}

	for (index = 1; index < binary_port_count; index++) {
		USERD->set_binary_manager(index, this);
	}
}

private void unregister_with_klib_userd()
{
	mixed *status;
	int index;

	status = status();

	USERD->set_telnet_manager(0, nil);

	for (index = 0; index < telnet_port_count; index++) {
		USERD->set_telnet_manager(index, nil);
	}

	for (index = 0; index < binary_port_count; index++) {
		USERD->set_binary_manager(index, nil);
	}

	telnet_port_count = 0;
	binary_port_count = 0;
}

static void timeout(object conn)
{
	if (conn->query_user() == this_object()) {
		connection(conn);
		disconnect();
	}
}

private object get_manager(object conn)
{
	object base;
	int port;

	base = conn;

	while (base <- LIB_USER) {
		base = base->query_conn();
	}

	port = base->query_port();

	if (base <- TELNET_CONN) {
		return telnet_managers[port];
	}

	if (base <- BINARY_CONN) {
		return binary_managers[port];
	}
}

/* external */

int free_users()
{
	return status()[ST_UTABSIZE] - sizeof(userd::query_connections());
}

void enable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	register_with_klib_userd();
	enabled = 1;
}

void disable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	enabled = 0;
	unregister_with_klib_userd();
}

void block_connections()
{
	object *conns;
	int index;

	ACCESS_CHECK(SYSTEM() || KADMIN());

	if (blocked) {
		blocked++;
		return;
	}

	LOGD->post_message("userd", LOG_NOTICE, "Blocking connections");

	blocked = 1;
	reblocked = ([ ]);

	conns = userd::query_connections();

	for (index = 0; index < sizeof(conns); index++) {
		object conn;

		conn = conns[index];

		/* don't interfere with the failsafe */
		if (conn->query_user() <- "/kernel/obj/user") {
			continue;
		}

		if (conn->query_mode() == MODE_BLOCK) {
			reblocked[conn] = 1;
		} else {
			conn->set_mode(MODE_BLOCK);
		}
	}
}

void unblock_connections()
{
	object *conns;
	int index;

	ACCESS_CHECK(SYSTEM() || KADMIN());

	if (blocked == 0) {
		error("Connections not blocked");
	}

	if (blocked > 1) {
		blocked--;
		return;
	}

	LOGD->post_message("userd", LOG_NOTICE, "Unblocking connections");

	blocked = 0;
	conns = userd::query_connections();

	for (index = 0; index < sizeof(conns); index++) {
		object conn;
		conn = conns[index];

		if (reblocked[conn]) {
			reblocked[conn] = nil;
		} else {
			conn->set_mode(MODE_UNBLOCK);
		}
	}

	reblocked = nil;
}

void reboot()
{
	if (enabled) {
		unregister_with_klib_userd();
		register_with_klib_userd();
	}
}

void set_binary_manager(int port, object LIB_USERD manager)
{
	ACCESS_CHECK(PRIVILEGED());

	PERMISSION_CHECK(port != 0);

	binary_managers[port] = manager;
}

void set_telnet_manager(int port, object LIB_USERD manager)
{
	ACCESS_CHECK(PRIVILEGED());

	telnet_managers[port] = manager;
}

void set_fixed_manager(int port, object LIB_USERD manager)
{
	ACCESS_CHECK(PRIVILEGED());

	fixed_managers[port] = manager;
}

/* initd hooks */

void prepare_reboot()
{
	ACCESS_CHECK(SYSTEM());

	unregister_with_klib_userd();
}

void bogus_reboot()
{
	ACCESS_CHECK(SYSTEM());

	register_with_klib_userd();
}

/* userd hooks */

string query_banner(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == USERD || SYSTEM());

	if (stacking) {
		/* stacking in progress */
		return nil;
	} else if (!(connection <- LIB_USER)) {
		/* first level, ignore */
		return nil;
	} else {
		/* stack built and we're not naked */
		/* it is safe to call the lieutenant */
		object userd;

		userd = get_manager(connection);

		if (!userd) {
			return "Internal error: no connection manager";
		}

		return userd->query_banner(connection);
	}
}

int query_timeout(object LIB_CONN connection)
{
	ACCESS_CHECK(previous_program() == USERD || SYSTEM());

	if (stacking) {
		/* stacking in progress */
		return 0;
	} else if (!(connection <- LIB_USER)) {
		/* first level, ignore */
		stacking = 1;
		connection(connection);
		redirect(clone_object("~/obj/filter/rlimits"), nil);
		return 0;
	} else {
		/* stack built and we're not naked */
		/* it is safe to call the lieutenant */
		object userd;

		userd = get_manager(connection);

		if (!userd) {
			return -1;
		}

		return userd->query_timeout(connection);
	}
}

object select(string str)
{
	object connection;
	/* do not call to lieutenant until after */
	/* connection stack is built */

	ACCESS_CHECK(previous_program() == USERD || SYSTEM());
	connection = previous_object(1);

	if (stacking) {
		/* stacking in progress */
		ASSERT(str == nil);
		return this_object();
	} else if (!(connection <- LIB_USER)) {
		/* first level, ignore */
		return this_object();
	} else {
		/* stack built and we're not naked */
		/* it is safe to call the lieutenant */
		object userd;

		userd = get_manager(connection);

		if (!userd) {
			return this_object();
		}

		return userd->select(str);
	}
}

/* connection hooks */

int login(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	if (stacking) {
		object conn;
		object base;
		object user;
		string banner;
		mapping managers;
		int port;
		int timeout;

		conn = previous_object();

		stacking = 0;

		banner = query_banner(conn);

		if (banner) {
			previous_object()->message(banner);
		}

		timeout = query_timeout(conn);

		if (timeout < -1) {
			return MODE_DISCONNECT;
		}

		call_out("timeout", timeout, conn);

		return MODE_NOCHANGE;
	} else {
		previous_object()->message("Internal error: connection manager fault\n");
		return MODE_DISCONNECT;
	}
}

int receive_message(string str)
{
	ACCESS_CHECK(previous_program() == LIB_CONN);

	connection(previous_object());
	redirect(select(str), str);

	return MODE_NOCHANGE;
}

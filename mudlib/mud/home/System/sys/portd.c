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

int enabled;		/*< active or not */
int binary_port_count;	/*< number of ports currently registered */
int telnet_port_count;	/*< number of ports currently registered */
int blocked;		/*< connection blocking is active */
mapping reblocked;	/*< objects that were already manually blocked */

/****************/
/* Declarations */
/****************/

static void create();

int login(string str);
void reboot();
void clear_ports();

void set_binary_manager(int port, object LIB_MANAGER manager);
void set_telnet_manager(int port, object LIB_MANAGER manager);
void set_fixed_manager(int port, object LIB_MANAGER manager);

int free_users();
void block_connections();
void unblock_connections();
int query_connection_count();
private atomic object query_select(string str, object conn);
private void register_with_klib_userd();
private void unregister_with_klib_userd();

/* basic */

static void create()
{
	user::create();
	userd::create();

	binary_managers = ([ ]);
	telnet_managers = ([ ]);
	fixed_managers = ([ ]);

	connections = ([ ]);

	reblocked = ([ ]);
}

/* admin */

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

	LOGD->post_message("system", LOG_NOTICE, "PortD: Blocking connections");

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

	LOGD->post_message("system", LOG_NOTICE, "PortD: Unblocking connections");

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

/* config */

void set_binary_manager(int port, object LIB_MANAGER manager)
{
	ACCESS_CHECK(PRIVILEGED());

	PERMISSION_CHECK(port != 0);

	binary_managers[port] = manager;
}

void set_telnet_manager(int port, object LIB_MANAGER manager)
{
	ACCESS_CHECK(PRIVILEGED());

	telnet_managers[port] = manager;
}

void set_fixed_manager(int port, object LIB_MANAGER manager)
{
	ACCESS_CHECK(PRIVILEGED());

	fixed_managers[port] = manager;
}

/*********/
/* Hooks */
/*********/

/* initd */

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

private object LIB_MANAGER manager_of(object LIB_CONN connection)
{
	mixed *cinfo;
	mapping checkme;
	object manager;

	cinfo = connections[connection];
	switch(cinfo[CINFO_TYPE]) {
	case "telnet":
		checkme = telnet_managers;
		break;

	case "binary":
		checkme = binary_managers;
		break;

	default:
		error("Unexpected connection type");
	}

	manager = checkme[cinfo[CINFO_LPORT]];

	if (!manager) {
		manager = fixed_managers[cinfo[CINFO_PPORT]];
	}

	return manager;
}

atomic string query_banner(object LIB_CONN connection)
{
	object base_conn;
	int level;
	object manager;

	ACCESS_CHECK(KERNEL());

	base_conn = connection;

	while (base_conn && base_conn <- LIB_USER) {
		base_conn = base_conn->query_conn();
		level++;

		if (level > 10) {
			error("Connection chain length overflow");
		}
	}

	if (!connections[base_conn]) {
		analyze_connection(base_conn);
	}

	manager = manager_of(base_conn);

	if (!manager) {
		return "Internal error:  No connection manager.\n"
	}

	catch {
		if (free_users() < SPARE_USERS) {
			return manager->query_overload_message(connection);
		} else if (blocked) {
			return manager->query_blocked_message(connection);
		} else {
			return manager->query_banner(connection);
		}
	} : {
		return "Internal error";
	}
}

atomic int query_timeout(object LIB_CONN connection)
{
	object base_conn;
	int level;
	object manager;

	ACCESS_CHECK(KERNEL());

	if (free_users() < SPARE_USERS || blocked) {
		return -1;
	}

	base_conn = connection;

	while (base_conn && base_conn <- LIB_USER) {
		base_conn = base_conn->query_conn();
		level++;

		if (level > 10) {
			error("Connection chain length overflow");
		}
	}

	manager = manager_of(base_conn);

	if (!manager) {
		return -1;
	}

	catch {
		int timeout;

		timeout = manager->query_timeout(connection);

		if (timeout == 0) {
			connection(connection);
			redirect(query_select(nil, connection), nil);
		}
	} : {
		return -1;
	}
}

atomic object select(string str)
{
	ACCESS_CHECK(KERNEL());

	return query_select(str, previous_object(1));
}

int login(string str)
{
	previous_object()->message("Internal error: connection manager fault\n");
	return MODE_DISCONNECT;
}

/********************/
/* Helper Functions */
/********************/

private atomic object query_select(string str, object conn)
{
	object base_conn;
	int level;
	object manager;
	object user;

	base_conn = conn;

	while (base_conn && base_conn <- LIB_USER) {
		base_conn = base_conn->query_conn();
		level++;

		if (level > 10) {
			error("Connection chain length overflow");
		}
	}

	manager = manager_of(base_conn);

	if (manager) {
		user = manager->select(str);
	}

	return user ? user : this_object();
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

int free_users()
{
	return status()[ST_UTABSIZE] - sizeof(userd::query_connections());
}

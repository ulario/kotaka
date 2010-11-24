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

/*************/
/* Variables */
/*************/

mapping telnet_managers;	/* managers assigned to logical telnet ports */
mapping binary_managers;	/* managers assigned to logical binary ports */
mapping fixed_managers;		/* managers assigned to physical ports */

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

private object LIB_MANAGER manager_of(object LIB_CONN connection)
{
	object cinfo;
	object manager;
	int lport;
	
	mapping checkme;
	
	cinfo = new_object(LWO_CINFO);
	cinfo->investigate(connection);
	
	switch(cinfo->query_conn_type()) {
	case "binary":
		checkme = binary_managers;
		break;
	case "telnet":
		checkme = telnet_managers;
		break;
	default:
		error("Unexpected connection type: " + cinfo->query_conn_type());
	}
	
	manager = checkme[cinfo->query_logical_port()];
	
	if (!manager) {
		manager = fixed_managers[cinfo->query_physical_port()];
	}
	
	if (!manager) {
		manager = find_object(DEFAULT_MANAGER);
	}
	
	return manager;
}

string query_banner(object LIB_CONN connection)
{
	object manager;
	
	ACCESS_CHECK(KERNEL());
	
	manager = manager_of(connection);
	
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

int query_timeout(object LIB_CONN connection)
{
	object manager;
	
	ACCESS_CHECK(KERNEL());
	
	if (free_users() < SPARE_USERS || blocked) {
		return -1;
	}
	
	manager = manager_of(connection);

	catch {
		int timeout;
		
		timeout = manager->query_timeout(connection);
		
		if (timeout == 0) {
			connection(connection);
			redirect(manager->select(nil), nil);
		}
	} : {
		return -1;
	}
}

object select(string str)
{
	object user;
	
	ACCESS_CHECK(KERNEL());
	
	catch {
		user = manager_of(previous_object(1))->select(str);

		if (user) {
			return user;
		}
	}
	
	return find_object(SYSTEM_DEFAULT_USER);
}

/********************/
/* Helper Functions */
/********************/

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

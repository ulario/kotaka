#include <status.h>

#include <kernel/user.h>

#include <kotaka/paths.h>
#include <kotaka/log.h>

inherit SECOND_AUTO;

object base_conn;
string conn_type;
int physical_port;
int logical_port;
int level;

static void create(int clone)
{
}

void investigate(object LIB_CONN new_conn)
{
	object conn;
	string path;
	int *checkme;
	
	conn = new_conn;
	conn_type = "void";
	physical_port = -1;
	logical_port = -1;
	level = 0;
	
	base_conn = conn;
	
	while (base_conn && base_conn <- LIB_USER) {
		base_conn = base_conn->query_conn();
		level++;
		
		if (level > 10) {
			error("Connection chain length overflow");
		}
	}
	
	if (!base_conn) {
		level = -1;
		return;
	}
	
	path = object_name(base_conn);
	sscanf(path, "%s#%*d", path);
	
	switch(path) {
	case TELNET_CONN:
		checkme = status()[ST_TELNETPORTS];
		conn_type = "telnet";
		break;
	case BINARY_CONN:
		checkme = status()[ST_BINARYPORTS];
		conn_type = "binary";
		break;
	default:
		error("Unrecognized connection type: " + path);
	}
	
	logical_port = base_conn->query_port();
	physical_port = checkme[logical_port];
}

object query_base_conn()
{
	return base_conn;
}

int query_physical_port()
{
	return physical_port;
}

int query_logical_port()
{
	return logical_port;
}

string query_conn_type()
{
	return conn_type;
}

int query_level()
{
	return level;
}

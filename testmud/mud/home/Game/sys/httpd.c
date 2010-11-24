#include <kernel/user.h>

#include <kotaka/assert.h>
#include <kotaka/paths.h>

#include <status.h>

inherit LIB_MANAGER;
inherit LIB_SYSTEM_USER;

mapping users;
mapping connections;

static void create()
{
	::create();
	
	connections = ([ ]);
	users = ([ ]);
	
	/* normal stuff */
	PORTD->set_fixed_manager(8080, this_object());
}

string query_overload_message()
{
	return
		"HTTP/1.1 500 Server busy\r\n"
	+	"Connection: close\r\n"
	+	"Retry-after: 60\r\n"
	+	"Content-Type: text\r\n"
	+	"\r\n"
	+	"Server overloaded\r\n";
}

string query_blocked_message()
{
	return
		"HTTP/1.1 500 Maintenance in progress\r\n"
	+	"Connection: close\r\n"
	+	"Retry-after: 20\r\n"
	+	"Content-Type: text\r\n"
	+	"\r\n"
	+	"Server undergoing maintenance\r\n";
}

string query_banner(object LIB_CONN connection)
{
	return
		"HTTP/1.1 200 OK\r\n"
	+	"Connection: close\r\n"
	+	"Content-Type: text\r\n"
	+	"\r\n"
	+	"Hello world\r\n";
}

int query_timeout(object LIB_CONN connection)
{
	return -1;
}

object select(string str)
{
	int level;
	object base_conn;
	
	base_conn = previous_object(2);
	
	while (base_conn && base_conn <- LIB_USER) {
		level++;
		base_conn = base_conn->query_conn();
	}
	
	if (!base_conn) {
		return nil;
	}
	
	if (level == 0) {
		return clone_object("~System/obj/filter/rlimits");
	}
	
	if (level == 1) {
		return this_object();
	}
}

int login(string str)
{
	return MODE_DISCONNECT;
}

#include <kernel/user.h>

#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>

#include <status.h>

inherit LIB_MANAGER;
inherit LIB_SYSTEM_USER;

mapping users;
mapping connections;
int splash;

object select(string str);

static void create()
{
	::create();

	connections = ([ ]);
	users = ([ ]);

	/* normal stuff */
	PORTD->set_binary_manager(1, this_object());
	PORTD->set_telnet_manager(0, this_object());
}

string query_banner(object LIB_CONN connection)
{
	string *files;
	string ansi;
	int sz;
	int level;
	object base_conn;

	base_conn = connection;

	while (base_conn && base_conn <- LIB_USER) {
		level++;
		base_conn = base_conn->query_conn();
	}

	if (!base_conn) {
		return "";
	}
	
	if (level == 0) {
		int rnd;
		
		files = get_dir("~/data/splash/telnet_banners/chars/*")[0];
		sz = sizeof(files);
		
		if (splash >= sz) {
			splash = 0;
		}
		
		ansi = read_file("~/data/splash/telnet_banners/ansi/" + files[splash]);
		
		if (!ansi) {
			ansi = STRINGD->simple_ansify(
				read_file("~/data/splash/telnet_banners/chars/" + files[splash]),
				read_file("~/data/splash/telnet_banners/fgcolor/" + files[splash]),
				read_file("~/data/splash/telnet_banners/bgcolor/" + files[splash])
			);
			
			write_file("~/data/splash/telnet_banners/ansi/" + files[splash], ansi);
		};
		
		splash++;
		
	
		return ansi;
	}
}

int query_timeout(object LIB_CONN connection)
{
	int level;
	object base_conn;

	base_conn = connection;

	while (base_conn && base_conn <- LIB_USER) {
		level++;
		base_conn = base_conn->query_conn();
	}

	if (!base_conn) {
		return -1;
	}
	
	if (level == 0) {
		connection(connection);
		::set_mode(MODE_RAW);
		redirect(select(nil), nil);
	}
	
	return 1;
}

object select(string str)
{
	int level;
	object base_conn;
	string basename;

	base_conn = previous_object(2);

	while (base_conn && base_conn <- LIB_USER) {
		level++;
		base_conn = base_conn->query_conn();
	}

	if (!base_conn) {
		return nil;
	}
	
	sscanf(object_name(base_conn), "%s#%*d", basename);
	
	switch(basename) {
	case BINARY_CONN:
		switch(level) {
		case 0: return clone_object("~System/obj/filter/rlimits");
		case 1: return clone_object("~Game/obj/filter/telnet");
		case 2: return clone_object("~Game/obj/user");
		}
	case TELNET_CONN:
		switch(level) {
		case 0: return clone_object("~System/obj/filter/rlimits");
		case 1: return clone_object("~Game/obj/user");
		}
	}
}

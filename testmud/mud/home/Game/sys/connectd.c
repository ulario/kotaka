#include <kernel/user.h>

#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>

#include <status.h>

inherit LIB_USERD;
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
	SYSTEM_USERD->set_telnet_manager(0, this_object());
	SYSTEM_USERD->set_binary_manager(1, this_object());
}

string query_blocked_banner(object LIB_CONN connection)
{
	return "Sorry, but due to system maintenance you cannot log in right now.\n";
}

string query_overload_banner(object LIB_CONN connection)
{
	return "There are too many connections to this mud.\n";
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

	if (level == 1) {
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

	if (level == 1) {
		connections[connection] = 1;
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
	object user;
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
		if (previous_object(2) <- "~Game/obj/filter/telnet") {
			user = clone_object("~Game/obj/user");
			users[user] = 1;
			return user;
		} else {
			return clone_object("~Game/obj/filter/telnet");
		}
	case TELNET_CONN:
		user = clone_object("~Game/obj/user");
		users[user] = 1;
		return user;
	}
}

object *query_connections()
{
	ACCESS_CHECK(PRIVILEGED());

	return map_indices(connections);
}

object *query_users()
{
	return map_indices(users);
}

#include <kernel/user.h>

#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/log.h>
#include <kotaka/privilege.h>

#include <status.h>

inherit LIB_MANAGER;
inherit LIB_SYSTEM_USER;

mapping users;
mapping connections;
int splash;

object select(string str);
void message(string str, varargs object src);

static void create()
{
	connections = ([ ]);
	users = ([ ]);
	PORTD->set_telnet_manager(2, this_object());
}

string query_banner(object LIB_CONN connection)
{
	return "";
}

int query_timeout(object LIB_CONN connection)
{
	object irc;

	if (map_sizeof(connections) >= 2) {
		return -1;
	}

	connections[connection] = 1;
	LOGD->post_message("IRCstat", LOG_INFO, "Added another IRC connection");

	connection(connection);
	::set_mode(MODE_RAW);
	irc = select(nil);
	users[irc] = 1;

	redirect(irc, nil);

	return 1;
}

object select(string str)
{
	return clone_object("~/obj/test/irc");
}

void handle_command(string command, string prefix, string *params)
{
}

void message(string str, varargs object source)
{
	int sz;
	object *user;

	user = map_indices(users);
	user -= ({ source });

	LOGD->post_message("IRC", LOG_INFO, sizeof(user) + ": " + str);

	for (sz = sizeof(user) - 1; sz >= 0; sz--) {
		LOGD->post_message(
			"IRCuser" + sz,
			LOG_INFO,
			"" + typeof(user[sz])
		);

		user[sz]->message(str + "\n");
	}
}

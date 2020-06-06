#include <kotaka/paths/channel.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;
inherit "/lib/string/sprint";

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	object user;
	string name;
	string ip;

	user = query_user();
	name = user->query_username();

	if (user->query_class() < 3) {
		send_out("Only an admin can list connections for CommD.\n");
		return;
	}

	send_out(mixed_sprint("~Test/sys/commd"->query_conns()) + "\n");
}

#include <kotaka/paths/channel.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

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
		send_out("Only an admin can unlink from another CommD.\n");
		return;
	}

	ip = roles["raw"];

	if (!ip) {
		send_out("Usage: cunlink <ip>\n");
		return;
	}

	"~Test/sys/commd"->unlink(ip);
}

#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	string *topics;

	if (query_user()->query_class() != 3) {
		send_out("Only a wizard can test the help system.\n");
		return;
	}

	topics = "~/sys/helpd"->query_topics(args);

	if (!sizeof(topics)) {
		send_out("There are no topics with that key");
	}

	send_out("Listed topics:\n\n");
	send_out(implode(topics, "\n") + "\n\n");
}

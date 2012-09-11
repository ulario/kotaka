#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	string *topics;
	string *categories;

	topics = "~/sys/helpd"->query_topics(args);

	if (topics) {
		categories = "~/sys/helpd"->query_categories(args);

		if (sizeof(categories)) {
			send_out("Categories: " + implode(categories, ", ") + "\n");
		}
		if (sizeof(topics)) {
			send_out("Topics: " + implode(topics, ", ") + "\n");
		}
	} else {
		send_out("No such category.\n");
		return;
	}
}

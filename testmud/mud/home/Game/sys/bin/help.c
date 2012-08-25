#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	string topic;
	string *options;

	args = STRINGD->replace(args, " ", "_");

	if (args == "") {
		topic = "index";
	} else {
		topic = args;
	}

	options = "~/sys/helpd"->query_topics(topic);

	if (!sizeof(options)) {
		send_out("No such topic.\n");
		return;
	}

	if (sizeof(options) > 1) {
		send_out("Be more specific, we have:\n\n");
		send_out(implode(options, "\n") + "\n\n");
		return;
	}

	pager = clone_object("~Kotaka/obj/ustate/page");
	pager->set_text(read_file("~/data/help/" + options[0] + ".hlp"));
	query_ustate()->push_state(pager);
}

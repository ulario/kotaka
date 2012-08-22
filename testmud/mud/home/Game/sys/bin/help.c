#include <kotaka/paths.h>
#include <game/paths.h>

inherit LIB_BIN;

void main(string args)
{
	string topic;
	object pager;
	mixed *info;

	if (args == "") {
		switch(query_user()->query_class()) {
		case 3:
			topic = "admin/index";
			break;
		case 2:
			topic = "wizard/index";
			break;
		case 1:
			topic = "player/index";
			break;
		case 0:
			topic = "guest/index";
			break;
		}
	} else {
		topic = args;
	}

	info = file_info("~Game/data/help/" + topic + ".hlp");

	if (!info) {
		send_out(topic + ": No such help topic.\n");
		return;
	}

	if (info[0] < 0) {
		send_out(topic + ": Is a category.\n");
		return;
	}

	pager = clone_object("~Kotaka/obj/ustate/page");
	pager->set_text(read_file("~Game/data/help/" + topic + ".hlp"));
	query_ustate()->push_state(pager);
}

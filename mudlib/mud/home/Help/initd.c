#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/bigstruct.h>
#include <kotaka/log.h>

inherit LIB_INITD;
inherit UTILITY_COMPILE;

static void create()
{
	load_dir("obj", 1);
	load_dir("sys", 1);
}

void full_reset()
{
	object turkeylist;
	object cursor;
	object first;
	object this;

	ACCESS_CHECK(PRIVILEGED());

	turkeylist = new_object(BIGSTRUCT_DEQUE_LWO);

	this = this_object();
	cursor = KERNELD->first_link("Help");
	first = cursor;

	do {
		LOGD->post_message("help", LOG_DEBUG, "Listing " + object_name(cursor));
		turkeylist->push_back(cursor);
		cursor = KERNELD->next_link(cursor);
	} while (cursor != first);

	while (!turkeylist->empty()) {
		object turkey;

		turkey = turkeylist->get_front();
		turkeylist->pop_front();

		if (!turkey || turkey == this) {
			/* don't self destruct or double destruct */
			continue;
		}

		destruct_object(turkey);
	}

	load_dir("obj", 1);
	load_dir("sys", 1);
}

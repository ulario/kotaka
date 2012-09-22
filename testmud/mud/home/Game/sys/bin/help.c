#include <kotaka/paths.h>
#include <game/paths.h>
#include <kotaka/assert.h>

inherit LIB_BIN;

int topic_compare(string lhs, string rhs)
{
	string *lparts, *rparts;
	int lsz, rsz;
	int csz, i;

	ASSERT(lhs && rhs);

	lparts = explode(lhs, "/");
	rparts = explode(rhs, "/");

	lsz = sizeof(lparts);
	rsz = sizeof(rparts);

	csz = lsz < rsz ? lsz : rsz;
	csz--;

	for (i = 0; i < csz; i++) {
		if (lparts[i] != rparts[i]) {
			return 0; /* forked */
		}
	}

	ASSERT(lsz != rsz);

	return lsz < rsz ? -1 : 1;
}

string *topic_filter(string *topics)
{
	int i;
	int sz;

	string *contenders;

	contenders = ({ topics[0] });

	sz = sizeof(topics);

	for (i = 1; i < sz; i++) {
		int j;
		int sz2;
		int survive;
		survive = 1;

		sz2 = sizeof(contenders);

		for (j = 0; j < sz2; j++) {
			switch(topic_compare(topics[i], contenders[j])) {
			case -1: /* victory */
				contenders[j] = nil;
				/* fall through */
			case 0: /* tie */
				break;
			case 1: /* defeat */
				survive = 0;
				continue;
			}
		}

		contenders -= ({ nil });

		if (survive) {
			contenders += ({ topics[i] });
		}
	}

	return contenders;
}

void main(string args)
{
	mapping index;
	mapping list;
	string *candidates;
	string *survivors;
	string topic;
	int sz, i;

	args = STRINGD->trim_whitespace(args);
	args = STRINGD->replace(args, " ", "_");

	if (args == "") {
		args = "index";
	}

	index = HELPD->query_index();
	ASSERT(index);

	if (index[args] == nil) {
		send_out("No such topic or category is available.\n");
		return;
	}

	list = index[args];
	candidates = map_indices(list);
	sz = sizeof(candidates);

	survivors = topic_filter(candidates);
	sz = sizeof(survivors);

	if (sz == 1) {
		object paint;
		object pager;
		string text;
		string header;

		pager = clone_object("~Kotaka/obj/ustate/page");
		paint = new_object("~Kotaka/lwo/painter");

		topic = survivors[0];

		if (list[topic] & 1) {
			header = topic;
			text = HELPD->query_content(topic);
		} else {
			if (HELPD->test_topic(topic + "/index")) {
				header = topic + "/index";
				text = HELPD->query_content(topic + "/index");
			} else {
				string *chunks;
				string *categories;
				string *topics;

				header = topic + " (contents)";

				chunks = ({ });

				categories = HELPD->query_categories(topic);
				topics = HELPD->query_topics(topic);

				if (sizeof(categories)) {
					chunks += ({ "Categories:\n    " + implode(categories, "\n    ") });
				}

				if (sizeof(topics)) {
					chunks += ({ "Topics:\n    " + implode(topics, "\n    ") });
				}

				if (!sizeof(categories) && !sizeof(topics)) {
					chunks += ({ "...nothing" });
				}

				text = implode(chunks, "\n\n");
			}
		}

		if (sizeof(candidates - survivors)) {
			text += "\nSee also: " + implode(candidates - survivors, ", ") + "\n";
		}

		text = "[ " + header + " ]\n\n" + text;
		pager->set_text(text);

		query_ustate()->push_state(pager);
		return;
	} else {
		send_out("That topic is ambiguous, please choose from:\n");

		for (i = 0; i < sz; i++) {
			send_out(survivors[i] + "\n");
		}
	}
}

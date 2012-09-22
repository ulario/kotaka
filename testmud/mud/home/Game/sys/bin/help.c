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

private string list_category(string category)
{
	string *chunks;
	string *categories;
	string *topics;
	string text;

	chunks = ({ });

	categories = HELPD->query_categories(category);
	topics = HELPD->query_topics(category);

	if (sizeof(categories)) {
		chunks += ({ "Categories:\n    " + implode(categories, "\n    ") });
	}

	if (sizeof(topics)) {
		chunks += ({ "Topics:\n    " + implode(topics, "\n    ") });
	}

	if (!sizeof(categories) && !sizeof(topics)) {
		chunks += ({ "...nothing" });
	}

	return implode(chunks, "\n\n");
}

void main(string args)
{
	mapping index;
	mapping list;
	string *candidates;
	string *survivors;
	string topic;
	string header;
	string text;
	int sz, i;

	args = STRINGD->trim_whitespace(args);
	args = STRINGD->replace(args, " ", "_");

	if (args == "") {
		/* list root category */
		if (HELPD->test_topic("index")) {
			header = "index";
			text = HELPD->query_content("index");
		} else {
			header = "(root contents)";
			text = list_category(nil);
		}
	} else {
		/* find topic, barf if ambiguous */
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
			topic = survivors[0];

			if (list[topic] & 1) {
				header = topic;
				text = HELPD->query_content(topic);
			} else {
				if (HELPD->test_topic(topic + "/index")) {
					header = topic + "/index";
					text = HELPD->query_content(topic + "/index");
				} else {
					header = topic + " (contents)";
					text = list_category(topic);
				}
			}

			if (sizeof(candidates - survivors)) {
				text += "\nSee also: " + implode(candidates - survivors, ", ") + "\n";
			}
		} else {
			send_out("That topic is ambiguous, please choose from:\n");

			for (i = 0; i < sz; i++) {
				send_out(survivors[i] + "\n");
			}
		}
	}

	if (header) {
		object pager;
		pager = clone_object("~Kotaka/obj/ustate/page");
		pager->set_text("[ " + header + " ]\n\n" + text);
		query_ustate()->push_state(pager);
	}
}

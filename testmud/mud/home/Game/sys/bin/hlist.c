#include <kotaka/paths.h>
#include <game/paths.h>
#include <kotaka/assert.h>

inherit LIB_BIN;

string *topic_mask(string *topics, int *bits, int mask)
{
	int sz, i;
	sz = sizeof(topics);

	topics = topics[..];

	for (i = 0; i < sz; i++) {
		if (!(bits[i] & mask)) {
			topics[i] = nil;
		}
	}

	topics -= ({ nil });

	return topics;
}

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
		candidates = survivors = ({ "" });
		sz = 1;
	} else {
		index = "~/sys/helpd"->query_index("");
		ASSERT(index);

		if (index[args] == nil) {
			send_out("No such category is available.\n");
			return;
		}

		list = index[args];
		candidates = topic_mask(map_indices(list), map_values(list), 2);
		sz = sizeof(candidates);

		if (!sz) {
			send_out("No such category is available.\n");
			return;
		}

		if (sizeof(candidates)) {
			send_out("Candidates: " + implode(candidates, ", ") + "\n\n");
		}

		survivors = topic_filter(candidates);
		sz = sizeof(survivors);
	}

	if (sz == 1) {
		object pager;
		string text;
		string *categories;
		string *topics;

		pager = clone_object("~Kotaka/obj/ustate/page");

		topic = survivors[0];

		if (topic == "") {
			text = "Top level contents:\n\n";
		} else {
			text = "Contents of " + topic + ":\n\n";
		}

		categories = "~/sys/helpd"->query_categories(topic);
		topics = "~/sys/helpd"->query_topics(topic);

		if (sizeof(categories)) {
			text += "Categories: " + implode(categories, ", ") + "\n\n";
		}

		if (sizeof(topics)) {
			text += "Topics: " + implode(topics, ", ") + "\n\n";
		}

		if (!sizeof(categories) && !sizeof(topics)) {
			text += "...nothing\n\n";
		}

		if (sizeof(candidates - survivors)) {
			text += "\nSee also: " + implode(candidates - survivors, ", ") + "\n";
		}

		pager->set_text(text);

		query_ustate()->push_state(pager);
		return;
	} else {
		send_out("That category is ambiguous, please choose from:\n");

		for (i = 0; i < sz; i++) {
			send_out(survivors[i] + "\n");
		}
	}
}

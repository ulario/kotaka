/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014, 2015  Raymond Jennings
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <kotaka/assert.h>
#include <kotaka/paths/help.h>
#include <kotaka/paths/string.h>
#include <kotaka/paths/verb.h>

inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "raw" });
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

mapping deep_list(string category)
{
	mapping submap;

	string *topics;
	string *categories;
	int sz, i;

	categories = HELPD->query_categories(category);
	topics = HELPD->query_topics(category);

	submap = ([ ]);

	sz = sizeof(categories);

	if (!category) {
		for (i = 0; i < sz; i++) {
			string subcategory;

			subcategory = categories[i];

			submap += deep_list(subcategory);
		}
	} else {
		for (i = 0; i < sz; i++) {
			string subcategory;

			subcategory = category + "/" + categories[i];

			submap += deep_list(subcategory);
		}
	}

	submap[category] = topics;

	return submap;
}

private string list_category(string category)
{
	string buf;
	string *categories;
	mapping list;
	int sz, i;

	list = deep_list(category);
	categories = map_indices(list);
	sz = sizeof(categories);

	buf = "";

	for (i = 0; i < sz; i++) {
		string *topics;
		string subcategory;
		string line;

		subcategory = categories[i];
		topics = list[subcategory];

		if (!sizeof(topics)) {
			continue;
		}

		if (subcategory) {
			buf += subcategory + ":\n";
		} else {
			buf += "Root:\n";
		}

		line = implode(topics, ",");
		line = STRINGD->replace(line, " ", "_");
		line = STRINGD->replace(line, ",", ", ");
		line = STRINGD->wordwrap(line, 72);
		line = STRINGD->replace(line, "\n", "\n    ");
		buf += "    " + line + "\n\n";
	}

	return buf;
}

void main(object actor, mapping roles)
{
	mapping index;
	mapping list;
	string *candidates;
	string *survivors;
	string topic;
	string header;
	string text;
	int sz, i;

	roles["raw"] = STRINGD->trim_whitespace(roles["raw"]);
	roles["raw"] = STRINGD->replace(roles["raw"], " ", "_");

	if (roles["raw"] == "") {
		/* list root category */
		header = "Root contents:";
		text = list_category(nil);
	} else {
		/* find topic, barf if ambiguous */
		index = HELPD->query_index();
		ASSERT(index);

		if (index[roles["raw"]] == nil) {
			send_out("No such topic or category is available.\n");
			return;
		}

		list = index[roles["raw"]];
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
				header = topic + " contents";
				text = list_category(topic);
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
		pager = new_object("~Text/lwo/ustate/page");
		pager->set_text("[\033[1;34m " + header + "\033[0m ]\n\n" + text);
		query_ustate()->push_state(pager);
	}
}

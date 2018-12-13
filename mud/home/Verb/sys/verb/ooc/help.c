/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
#include <kotaka/paths/verb.h>

#define HELPD "~Kotaka/sys/helpd"

inherit "/lib/string/replace";
inherit "/lib/string/format";
inherit "~System/lib/struct/list";
inherit LIB_VERB;

private void gather_categories(string category, mixed **list)
{
	int sz, i;
	string *categories;

	categories = HELPD->query_categories(category);
	sz = sizeof(categories);

	if (category) {
		for (i = 0; i < sz; i++) {
			string subcategory;

			subcategory = category + "/" + categories[i];

			list_push_back(list, subcategory);

			gather_categories(subcategory, list);
		}
	} else {
		for (i = 0; i < sz; i++) {
			string subcategory;

			subcategory = categories[i];

			list_push_back(list, subcategory);

			gather_categories(subcategory, list);
		}
	}
}

private void list_category(string category)
{
	mixed **list;

	list = ({ nil, nil });
	list_push_back(list, category);

	gather_categories(category, list);

	while (!list_empty(list)) {
		string subcategory;
		string *topics;

		subcategory = list_front(list);
		list_pop_front(list);

		topics = HELPD->query_topics(subcategory);

		if (sizeof(topics)) {
			if (subcategory) {
				send_out("\033[1m" + subcategory + ":\033[0m\n");
			}
			send_out("    " + implode(topics, ", ") + "\n\n");
		}
	}
}

private string *filter_topics(string *topics)
{
	string *filters;
	int i, j;
	int sz;

	filters = topics[..];
	sz = sizeof(filters);

	for (i = 0; i < sz; i++) {
		int sz2;

		sz2 = sizeof(topics);

		for (j = 0; j < sz2; j++) {
			if (sscanf(topics[j], filters[i] + "/%*s")) {
				topics[j] = nil;
			}
		}

		topics -= ({ nil });
	}

	return topics;
}

string *query_parse_methods()
{
	return ({ "raw" });
}

void main(object actor, mapping roles)
{
	string topic;

	topic = roles["raw"];

	topic = trim_whitespace(topic);
	topic = replace(topic, " ", "_");

	if (topic == "") {
		list_category(nil);
	} else {
		mapping index;
		int test;
		string *topics;
		int i, sz;

		index = HELPD->query_index(topic);

		if (!index) {
			send_out("No such topic or category\n");
			return;
		}

		topics = map_indices(index);

		topics = filter_topics(topics);

		sz = sizeof(topics);

		if (sz > 1) {
			send_out("Topics:\n\n");

			for (i = 0; i < sz; i++) {
				string topic;

				topic = topics[i];

				send_out(topic);

				if (HELPD->test_entry(topic) == 2) {
					send_out(" (category)");
				}

				send_out("\n");
			}

			return;
		}

		topic = topics[0];
		test = HELPD->test_entry(topic);

		switch(test) {
		case 1:
			{
				string content;

				content = HELPD->query_topic(topic);

				send_out("\033[1;35m--- " + topic + " ---\033[0m\n\n");
				send_out(content + "\n");
			}
			break;

		case 2:
			send_out("\033[1;34m--- " + topic + " ---\033[0m\n\n");
			list_category(topic);
		}
	}
}

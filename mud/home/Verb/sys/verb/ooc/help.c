/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2021  Raymond Jennings
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

	if (category) {
		send_out("[ \033[1;34m" + category + "\033[0m ]\n\n");
	} else {
		send_out("< \033[1;34mTable of contents\033[0m >\n\n");
	}

	while (!list_empty(list)) {
		string subcategory;
		string *topics;

		subcategory = list_front(list);
		list_pop_front(list);

		topics = HELPD->query_topics(subcategory);

		if (sizeof(topics)) {
			string line;

			if (subcategory) {
				send_out("\033[1m" + subcategory + ":\033[0m\n");
			}

			line = implode(topics, ", ");
			line = wordwrap_line(line, 60);
			line = "    " + replace(line, "\n", "\n    ");

			send_out(line + "\n\n");
		}
	}
}

private void show_topic(string topic)
{
	string content;

	content = HELPD->query_topic(topic);

	send_out("[ \033[1;34m" + topic + "\033[0m ]\n\n");

	send_out(content);
}

private string *filter_topics(string *topics)
{
	int i, j, sz;

	i = 0;

	while (i < sizeof(topics)) {
		int sz;

		for (sz = sizeof(topics); --sz > i; ) {
			if (sscanf(topics[sz], topics[i] + "/%*s")) {
				topics[sz] = nil;
			}
		}

		topics -= ({ nil });
		i++;
	}

	sz = sizeof(topics);

	for (i = 0; i < sz; i++) {
		if (!topics[i]) {
			continue;
		}

		for (j = 0; j < sz; j++) {
			if (i == j) {
				continue;
			}

			if (!topics[j]) {
				continue;
			}

			if (sscanf(topics[i], topics[j] + "/%*s")) {
				topics[i] = nil;
			} else if (sscanf(topics[i], "%*s/" + topics[j])) {
				topics[i] = nil;
			}
		}
	}

	topics -= ({ nil });

	return topics;
}

string *query_parse_methods()
{
	return ({ "raw" });
}

string query_help_title()
{
	return "Help";
}

string *query_help_contents()
{
	return ({ "Accesses the help system." });
}

void main(object actor, mapping roles)
{
	string topic;

	topic = roles["raw"];

	if (!topic) {
		list_category(nil);
	} else {
		int i, sz, test;
		mapping index;
		string *topics;

		topic = trim_whitespace(topic);
		topic = replace(topic, " ", "_");

		index = HELPD->query_index(topic);

		if (!index) {
			send_out("No such topic or category\n");
			return;
		}

		topics = map_indices(index);

		topics = filter_topics(topics);

		sz = sizeof(topics);

		if (sz > 1) {
			send_out("That topic is ambiguous, please choose from the following topics:\n\n");

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
			show_topic(topic);
			break;

		case 2:
			list_category(topic);
			break;
		}
	}
}

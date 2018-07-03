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
#include <kotaka/privilege.h>
#include <kotaka/assert.h>

object root_category;

static void create()
{
	root_category = clone_object("~/obj/category");
}

static void destruct()
{
	if (root_category) {
		destruct_object(root_category);
	}
}

void reset()
{
	ACCESS_CHECK(PRIVILEGED());

	destruct_object(root_category);

	root_category = clone_object("~/obj/category");
}

void add_category(string category)
{
	ACCESS_CHECK(PRIVILEGED());

	root_category->insert_entry(category, 1);
}

void add_topic(string topic, varargs mixed content)
{
	ACCESS_CHECK(PRIVILEGED());

	ASSERT(content);

	root_category->insert_entry(topic, 0)->set_content(content);
}

void remove_category(string category)
{
	ACCESS_CHECK(PRIVILEGED());

	root_category->delete_entry(category, 1);
}

void remove_topic(string topic)
{
	ACCESS_CHECK(PRIVILEGED());

	root_category->delete_entry(topic, 0);
}

string *query_topics(varargs string category)
{
	object subnode;

	if (!category || category == "") {
		subnode = root_category;
	} else {
		subnode = root_category->find_node(category);
	}

	if (subnode) {
		return subnode->query_topics();
	} else {
		return nil;
	}
}

string *query_categories(varargs string category)
{
	object subnode;

	if (!category || category == "") {
		subnode = root_category;
	} else {
		subnode = root_category->find_node(category);
	}

	if (subnode) {
		return subnode->query_categories();
	} else {
		return nil;
	}
}

mapping query_index(varargs string category)
{
	object subnode;

	if (!category || category == "") {
		subnode = root_category;
	} else {
		subnode = root_category->find_node(category);
	}

	if (subnode) {
		mapping index;

		string *ind;
		mapping *val;
		int sz, i;

		index = subnode->query_index()[..];

		ind = map_indices(index);
		val = map_values(index);

		for (sz = map_sizeof(index), i = 0; i < sz; i++) {
			index[ind[i]] = val[i][..];
		}

		return index;
	} else {
		return nil;
	}
}

int test_topic(string topic)
{
	string *parts;
	string category;
	object subnode;
	int sz;

	parts = explode(topic, "/");

	if ((sz = sizeof(parts)) > 1) {
		topic = parts[sz - 1];
		subnode = root_category->find_node(implode(parts[0 .. sz - 2], "/"));
	} else {
		subnode = root_category;
	}

	if (!subnode) {
		return 0;
	}

	subnode = subnode->query_topic(topic);

	return !!subnode;
}

mixed query_content(string topic)
{
	string *parts;
	string category;
	object subnode;
	int sz;

	parts = explode(topic, "/");

	if ((sz = sizeof(parts)) > 1) {
		topic = parts[sz - 1];
		subnode = root_category->find_node(implode(parts[0 .. sz - 2], "/"));
	} else {
		subnode = root_category;
	}

	if (!subnode) {
		error("No such category");
	}

	subnode = subnode->query_topic(topic);

	if (!subnode) {
		error("No such topic");
	}

	return subnode->query_content();
}

void dump_topic(string topic)
{
	write_file("help/" + topic + ".hlp", query_content(topic));
}

void dump_category(varargs string category)
{
	int sz;
	string *categories;
	string *topics;

	if (category) {
		make_dir("help/" + category);
	} else {
		make_dir("help");
	}

	categories = query_categories(category);
	topics = query_topics(category);

	for (sz = sizeof(categories) - 1; sz >= 0; --sz) {
		if (category) {
			dump_category(category + "/" + categories[sz]);
		} else {
			dump_category(categories[sz]);
		}
	}

	for (sz = sizeof(topics) - 1; sz >= 0; --sz) {
		if (category) {
			dump_topic(category + "/" + topics[sz]);
		} else {
			dump_topic(topics[sz]);
		}
	}
}

void dump()
{
	dump_category(nil);
}

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
#include <type.h>

inherit "/lib/string/sprint";

mapping tree;
object index;

void reset();

private void add_index(string entry, string topic)
{
	mapping topics;

	topics = index->query_element(entry);

	if (!topics) {
		topics = ([ ]);
	}

	topics[topic] = 1;

	index->set_element(entry, topics);
}

private void remove_index(string entry, string topic)
{
	mapping topics;

	topics = index->query_element(entry);
	topics[topic] = nil;

	if (!map_sizeof(topics)) {
		topics = nil;
	}

	index->set_element(entry, topics);
}

private void index_topic(string *parts, int offset, string topic)
{
	if (offset >= 0) {
		index_topic(parts, offset - 1, topic);
		if (offset) {
			parts = parts[.. offset - 1] + parts[offset + 1 ..];
		} else {
			parts = parts[1 ..];
		}
		index_topic(parts, offset - 1, topic);
	} else {
		add_index(implode(parts, "/"), topic);
	}
}

private void deindex_topic(string *parts, int offset, string topic)
{
	if (offset >= 0) {
		deindex_topic(parts, offset - 1, topic);
		if (offset) {
			parts = parts[.. offset - 1] + parts[offset + 1 ..];
		} else {
			parts = parts[1 ..];
		}
		deindex_topic(parts, offset - 1, topic);
	} else {
		remove_index(implode(parts, "/"), topic);
	}
}

static void create()
{
	reset();
}

static void do_dump()
{
	remove_file("help-dump");
	write_file("help-dump", hybrid_sprint(tree) + "\n\n");
	write_file("help-dump", hybrid_sprint(index->query_root()) + "\n\n");
}

void add_topic(string topic, string content)
{
	string *parts;
	mapping map;
	int sz, i;

	if (topic == nil) {
		error("Invalid topic");
	}

	if (content == nil) {
		error("Invalid content");
	}

	parts = explode(topic, "/");

	map = tree;
	
	sz = sizeof(parts);

	for (i = 0; i < sz - 1; i++) {
		string part;

		part = parts[i];

		switch(typeof(map[part])) {
		case T_NIL:
			index_topic(parts[0 .. i], i - 1, implode(parts[0 .. i], "/"));
			map[part] = ([ ]);

		case T_MAPPING:
			map = map[part];
			break;

		case T_STRING:
			error("Topic in the way at " + implode(parts[0 .. i], "/"));
		}
	}

	map[parts[sz - 1]] = content;

	for (; sz > 0; sz--) {
		index_topic(parts[0 .. sz - 1], sz - 2, topic);
	}
}

void remove_topic(string topic)
{
	string *parts;
	mapping *map;
	int sz, i;

	if (topic == nil) {
		error("Invalid topic");
	}

	parts = explode(topic, "/");
	sz = sizeof(parts);

	map = allocate(sz);
	map[0] = tree;

	for (i = 0; i < sz - 1; i++) {
		string part;

		part = parts[i];

		map[i + 1] = map[i][part];

		switch(typeof(map[i][part])) {
		case T_NIL:
			error("No such category");

		case T_MAPPING:
			break;

		case T_STRING:
			error("Topic in the way");
		}
	}

	if (map[sz - 1][parts[sz - 1]] == nil) {
		error("No such topic");
	}

	map[sz - 1][parts[sz - 1]] = nil;

	while (i > 0) {
		if (map_sizeof(map[i]) == 0) {
			i--;
			deindex_topic(parts[0 .. i - 1], i - 2, implode(parts[0 .. i - 1], "/"));
			map[i][parts[i]] = nil;
		} else {
			break;
		}
	}
}

string query_topic(string topic)
{
	string *parts;
	mapping map;
	int sz, i;
	mixed content;

	parts = explode(topic, "/");

	map = tree;
	sz = sizeof(parts);

	for (i = 0; i < sz - 1; i++) {
		string part;

		part = parts[i];

		switch(typeof(map[part])) {
		case T_NIL:
			error("No such category");
			map[part] = ([ ]);

		case T_MAPPING:
			map = map[part];
			break;

		case T_STRING:
			error("Topic in the way");
		}
	}

	content = map[parts[sz - 1]];

	switch(typeof(content)) {
	case T_NIL:
		error("No such topic");

	case T_MAPPING:
		error("Is category");
	}

	return content;
}

int test_entry(string entry)
{
	string *parts;
	mapping map;
	int sz, i;
	mixed content;

	parts = explode(entry, "/");

	map = tree;
	sz = sizeof(parts);

	for (i = 0; i < sz - 1; i++) {
		string part;

		part = parts[i];

		switch(typeof(map[part])) {
		case T_NIL:
			return 0;

		case T_MAPPING:
			map = map[part];
			break;

		case T_STRING:
			return -1;
		}
	}

	content = map[parts[sz - 1]];

	switch(typeof(content)) {
	case T_NIL:
		return 0;

	case T_MAPPING:
		return 2;
	}

	return 1;
}

string *query_topics(string category)
{
	string *parts;
	mapping map;
	int sz, i;
	string *keys;
	mapping topics;

	map = tree;

	if (category) {
		parts = explode(category, "/");

		sz = sizeof(parts);

		for (i = 0; i < sz; i++) {
			string part;

			part = parts[i];

			switch(typeof(map[part])) {
			case T_NIL:
				return nil;

			case T_MAPPING:
				map = map[part];
				break;

			case T_STRING:
				error("Topic in the way");
			}
		}
	}

	keys = map_indices(map);
	topics = ([ ]);

	for (sz = sizeof(keys); --sz >= 0; ) {
		string key;

		key = keys[sz];

		if (typeof(map[key]) == T_STRING) {
			topics[key] = 1;
		}
	}

	return map_indices(topics);
}

string *query_categories(string category)
{
	int sz, i;
	string *parts;
	mapping map;
	string *keys;
	mapping categories;

	map = tree;

	if (category) {
		parts = explode(category, "/");

		sz = sizeof(parts);

		for (i = 0; i < sz; i++) {
			string part;

			part = parts[i];

			switch(typeof(map[part])) {
			case T_NIL:
				return nil;

			case T_MAPPING:
				map = map[part];
				break;

			case T_STRING:
				error("Topic in the way");
			}
		}
	}

	keys = map_indices(map);
	categories = ([ ]);

	for (sz = sizeof(keys); --sz >= 0; ) {
		string key;

		key = keys[sz];

		if (typeof(map[key]) == T_MAPPING) {
			categories[key] = 1;
		}
	}

	return map_indices(categories);
}

mapping query_index(string topic)
{
	mapping map;

	map = index->query_element(topic);

	if (map) {
		return map[..];
	}
}

void dump()
{
	call_out("do_dump", 0);
}

void reset()
{
	tree = ([ ]);
	index = new_object("/lwo/struct/mapping");
	index->set_type(T_STRING);
}

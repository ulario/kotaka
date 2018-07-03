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
#include <kotaka/privilege.h>
#include <config.h>

/* a node in the help tree */

/* one node per category */

/* maintains indexing information for all topics and categories at or under
its location */

object parent;		/* parent node, nil if root */
string category;	/* our label in the parent, nil if root */
mapping categories;	/* ([ name : category object ]); */
mapping topics;		/* ([ name : topic object ]) */
mapping index;		/* ([ key : ([ entry : bits ]) ]), bit 1 = topic, bit 2 = category */

static void create(int create)
{
	if (create) {
		categories = ([ ]);
		topics = ([ ]);
		index = ([ ]);
	}
}

static void destruct(int create)
{
	if (create) {
		object *turkeys;
		int i, sz;

		turkeys = map_values(categories);

		for (sz = sizeof(turkeys), i = 0; i < sz; i++) {
			destruct_object(turkeys[i]);
		}

		turkeys = map_values(topics);

		for (sz = sizeof(turkeys), i = 0; i < sz; i++) {
			destruct_object(turkeys[i]);
		}
	}
}

/* setup */
void set_parent(object "category" new_parent)
{
	ASSERT(!parent);

	parent = new_parent;
}

void set_category(string new_category)
{
	ASSERT(!category);

	category = new_category;
}

/* node search */
object find_node(string category)
{
	string *parts;
	object subnode;
	int sz;

	parts = explode(category, "/");
	ASSERT((sz = sizeof(parts)) > 0);

	subnode = categories[parts[0]];

	if (subnode) {
		if (sz == 1) {
			return subnode;
		} else {
			return subnode->find_node(implode(parts[1 ..], "/"));
		}
	} else {
		return nil;
	}
}

string *query_topics()
{
	return map_indices(topics);
}

object query_topic(string topic)
{
	return topics[topic];
}

string *query_categories()
{
	return map_indices(categories);
}

mapping query_index()
{
	return index;
}

int is_empty()
{
	return !map_sizeof(topics) && !map_sizeof(categories);
}

/* insert/removal */
/* entry is relative to node's current location in help tree */

private void index_insert(string key, string entry, int is_category)
{
	mapping submap;

	submap = index[key];

	if (!submap) {
		index[key] = submap = ([ ]);
	}

	if (submap[entry]) {
		submap[entry] |= (1 << is_category);
	} else {
		submap[entry] = (1 << is_category);
	}
}

private void index_delete(string key, string entry, int is_category)
{
	mapping submap;

	submap = index[key];

	if (!submap) {
		return;
	}

	if (submap[entry]) {
		submap[entry] &= ~(1 << is_category);
		if (!submap[entry]) {
			submap[entry] = nil;
			if (!map_sizeof(submap)) {
				index[key] = nil;
			}
		}
	}
}

object insert_entry(string entry, int is_category)
{
	string *parts;
	object subnode;
	int sz;

	ACCESS_CHECK(HELP());

	parts = explode(entry, "/");
	ASSERT((sz = sizeof(parts)) > 0);

	if (sz > 1) {
		/* must go deeper */
		subnode = categories[parts[0]];
		ASSERT(subnode);

		return subnode->insert_entry(implode(parts[1 ..], "/"), is_category);
	} else {
		/* we are at the bottom */
		if (is_category) {
			ASSERT(!categories[parts[0]]);

			subnode = clone_object("category");
			subnode->set_parent(this_object());
			subnode->set_category(parts[0]);
			categories[parts[0]] = subnode;
		} else {
			ASSERT(!topics[parts[0]]);

			subnode = clone_object("topic");
			topics[parts[0]] = subnode;
		}

		index_insert(parts[0], parts[0], is_category);

		if (parent) {
			parent->index_entry(category, parts[0], parts[0], is_category);
		}

		return subnode;
	}
}

void delete_entry(string entry, int is_category)
{
	string *parts;
	object subnode;
	int sz;

	ACCESS_CHECK(HELP());

	parts = explode(entry, "/");
	ASSERT((sz = sizeof(parts)) > 0);

	if (sz > 1) {
		/* must go deeper */
		subnode = categories[parts[0]];
		ASSERT(subnode);

		subnode->delete_entry(implode(parts[1 ..], "/"), is_category);
	} else {
		/* we are at the bottom */
		if (is_category) {
			ASSERT(categories[parts[0]]);

			subnode = categories[parts[0]];

			ASSERT(subnode->is_empty());

			destruct_object(subnode);
		} else {
			ASSERT(topics[parts[0]]);

			destruct_object(topics[parts[0]]);
		}

		index_delete(parts[0], parts[0], is_category);

		if (parent) {
			parent->deindex_entry(category, parts[0], parts[0], is_category);
		}
	}
}

void index_entry(string subcategory, string key, string entry, int is_category)
{
	ACCESS_CHECK(HELP());

	index_insert(key, subcategory + "/" + entry, is_category);
	index_insert(subcategory + "/" + key, subcategory + "/" + entry, is_category);

	if (parent) {
		parent->index_entry(category, key, subcategory + "/" + entry, is_category);
		parent->index_entry(category, subcategory + "/" + key, subcategory + "/" + entry, is_category);
	}
}

void deindex_entry(string subcategory, string key, string entry, int is_category)
{
	ACCESS_CHECK(HELP());

	index_delete(key, subcategory + "/" + entry, is_category);
	index_delete(subcategory + "/" + key, subcategory + "/" + entry, is_category);

	if (parent) {
		parent->deindex_entry(category, key, subcategory + "/" + entry, is_category);
		parent->deindex_entry(category, subcategory + "/" + key, subcategory + "/" + entry, is_category);
	}
}

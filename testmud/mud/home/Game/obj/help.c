#include <kotaka/assert.h>
#include <kotaka/privilege.h>
#include <config.h>

/* a node in the help tree */

/* one node per category */

/* maintains indexing information for all topics and categories at or under
its location */

object parent;		/* parent node, nil if root */
mapping categories;	/* ([ name : subnode ]); */
mapping topics;		/* ([ name : 1 ]) */
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
	}
}

/* setup */
void set_parent(object "help" new_parent)
{
	ASSERT(!parent);

	parent = new_parent;
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

string *query_categories()
{
	return map_indices(categories);
}

/* insert/removal */
/* entry is relative to node's current location in help tree */

void insert_entry(string entry, int is_category)
{
	string *parts;
	object subnode;
	int sz;

	parts = explode(entry, "/");
	ASSERT((sz = sizeof(parts)) > 0);

	if (sz > 1) {
		/* must go deeper */
		subnode = categories[parts[0]];
		ASSERT(subnode);

		subnode->insert_entry(implode(parts[1 ..], "/"), is_category);
	} else {
		/* we are at the bottom */
		if (is_category) {
			ASSERT(!categories[parts[0]]);

			subnode = clone_object("help");
			subnode->set_parent(this_object());
			categories[parts[0]] = subnode;
		} else {
			ASSERT(!topics[parts[0]]);

			topics[parts[0]] = 1;
		}
	}
}

void delete_entry(string entry, int is_category)
{
	string *parts;
	object subnode;
	int sz;

	parts = explode(entry, "/");
	ASSERT((sz = sizeof(parts)) > 0);

	if (sz > 1) {
		/* must go deeper */
		subnode = categories[parts[0]];
		ASSERT(subnode);

		subnode->insert_entry(implode(parts[1 ..], "/"), is_category);
	} else {
		/* we are at the bottom */
		if (is_category) {
			ASSERT(categories[parts[0]]);

			subnode = categories[parts[0]];

			ASSERT(subnode->is_empty());

			destruct_object(subnode);
		} else {
			ASSERT(topics[parts[0]]);

			topics[parts[0]] = nil;
		}
	}
}

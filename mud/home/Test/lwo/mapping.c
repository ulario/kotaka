#include <type.h>
#include <kotaka/paths/string.h>
#include <kotaka/assert.h>
#include <kotaka/paths/system.h>
#include <kotaka/log.h>

#define MAX_BRANCH_SIZE 4
#define MAX_LEAF_SIZE   128

inherit "/lib/string/sprint";
inherit "/lib/search";

mixed root;
int type;

/* node structure: */

/* leaf node */
/* ({ size, ([ key : value ]) }) */

/* branch node */
/* ({ keys, ([ key : subnode ]) }) */

void set_type(int new_type)
{
	switch(new_type)
	{
	case T_INT:
	case T_FLOAT:
	case T_STRING:
		type = new_type;
		root = ({ 0, ([ ]) });
		return;

	default:
		error("Invalid type");
	}
}

private mixed split_node(mixed *parent, int chubby)
{
	mixed *keys;
	mixed key;
	mixed splitkey;
	mixed *node;
	mixed *subkeys;
	int sz;
	mapping lower;
	mapping upper;

	keys = parent[0];
	key = keys[chubby];

	node = parent[1][key];

	subkeys = map_indices(node[1]);
	sz = sizeof(subkeys);
	splitkey = subkeys[sz / 2];

	lower = node[1][.. splitkey];
	upper = node[1][splitkey ..];
	lower[splitkey] = nil;

	switch(typeof(node[0])) {
	case T_INT: /* splitting a leaf node */
		parent[1][key] = ({ map_sizeof(lower), lower });
		parent[1][splitkey] = ({ map_sizeof(upper), upper });
		break;

	case T_ARRAY: /* splitting a branch node */
		parent[1][key] = ({ subkeys[.. sz / 2 - 1], lower });
		parent[1][splitkey] = ({ subkeys[sz / 2 ..], upper });
		break;
	}

	if (chubby + 1 == sizeof(keys)) {
		keys += ({ splitkey });
	} else {
		keys = keys[.. chubby] + ({ splitkey }) + keys[chubby + 1 ..];
	}

	parent[0] = keys;

	return splitkey;
}

/* -1 = node too big */
private int sub_set_element(mixed *node, mixed key, mixed value)
{
	mixed head;
	mapping map;

	head = node[0];
	map = node[1];

	switch(typeof(head)) {
	case T_INT: /* leaf node */
		if (value != nil) {
			if (head >= MAX_LEAF_SIZE) {
				/* compact */
				head = map_sizeof(map);

				if (head >= MAX_LEAF_SIZE) {
					/* overflow */
					return -1;
				}
			}
		}

		if (map[key] != nil) {
			head--;
		}

		map[key] = value;

		if (map[key] != nil) {
			head++;
		}

		node[0] = head;
		break;

	case T_ARRAY: /* branch node */
		{
			int subindex;
			mixed subkey;
			mixed splitkey;
			mixed newkey;
			mixed *subnode;

			/* find floor */
			subindex = binary_search_floor(head, key);

			if (subindex == -1) {
				subindex = 0;
				newkey = key;
			}

			subkey = head[subindex];
			subnode = map[subkey];

			if (sub_set_element(subnode, key, value) == -1) {
				if (sizeof(head) > MAX_BRANCH_SIZE) {
					/* we're too big to be split, punt */
					return -1;
				}

				splitkey = split_node(node, subindex);

				if (key >= splitkey) {
					/* upper half */
					subnode = map[splitkey];
				}

				sub_set_element(subnode, key, value);
			}

			if (newkey != nil) {
				/* rebottom */
				map[subkey] = nil;
				map[newkey] = subnode;
				head[0] = newkey;
			}

			return 0;
		}
	}
}

void set_element(mixed key, mixed value)
{
	if (typeof(key) != type) {
		error("Type mismatch");
	}

	if (sub_set_element(root, key, value) == -1) {
		mixed head;
		mixed bottom;

		head = root[0];

		switch(typeof(head)) {
		case T_INT:
			head = map_indices(root[1]);
			break;

		case T_ARRAY:
			break;
		}

		bottom = head[0];

		root = ({
			({ bottom }),
			([ bottom : root ])
		});

		sub_set_element(root, key, value);
	}
}

private mixed sub_query_element(mixed *node, mixed key)
{
	mixed head;
	mapping map;

	head = node[0];
	map = node[1];

	switch(typeof(head)) {
	case T_INT:
		return map[key];

	case T_ARRAY:
		{
			int subindex;
			mixed subkey;
			mixed subnode;

			/* find floor */
			subindex = binary_search_floor(head, key);

			if (subindex == -1) {
				return -1;
			}

			subkey = head[subindex];
			subnode = map[subkey];

			return sub_query_element(subnode, key);
		}
	}
}

mixed query_element(mixed key)
{
	return sub_query_element(root, key);
}

mixed *query_root()
{
	return root;
}
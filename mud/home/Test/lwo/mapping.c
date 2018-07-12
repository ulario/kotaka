#include <type.h>
#include <kotaka/paths/string.h>
#include <kotaka/assert.h>
#include <kotaka/paths/system.h>
#include <kotaka/log.h>

#define MAX_BRANCH_SIZE 4
#define MAX_LEAF_SIZE   4

inherit "/lib/search";

mixed *root; /* ({ is_leaf, ({ keys }), ([ key : value/subnode ]) }) */
int type;

void set_type(int new_type)
{
	switch(new_type)
	{
	case T_INT:
	case T_FLOAT:
	case T_STRING:
		type = new_type;
		root = ({ 1, ([ ]) });
		return;

	default:
		error("Invalid type");
	}
}

private void split_node(mixed *supernode, int i)
{
	mixed *keys;
	mapping map;

	mixed key;

	mixed *subnode;
	mixed *subkeys;
	mapping submap;

	int subsz;

	mixed *lowkeys;
	mixed *highkeys;
	mixed midkey;

	mapping lowmap;
	mapping highmap;

	mixed *subnode1;
	mixed *subnode2;

	keys = supernode[1];
	map = supernode[2];

	key = keys[i];

	subnode = map[key];

	if (subnode[0]) {
		submap = subnode[1];
		subkeys = map_indices(submap);
	} else {
		subkeys = subnode[1];
		submap = subnode[2];
	}

	subsz = sizeof(subkeys);
	subsz /= 2;

	midkey = subkeys[subsz];

	if (subnode[0]) {
	} else {
		lowkeys = subkeys[.. subsz - 1];
		highkeys = subkeys[subsz ..];
	}

	lowmap = submap[.. midkey];
	highmap = submap[midkey ..];
	lowmap[midkey] = nil;

	if (subnode[0]) {
		subnode1 = ({ 1, lowmap });
		subnode2 = ({ 1, highmap });
	} else {
		subnode1 = ({ 0, lowkeys, lowmap });
		subnode2 = ({ 0, highkeys, highmap });
	}

	supernode[1] = keys[.. i] + ({ midkey }) + keys[i + 1 ..];

	map[key] = subnode1;
	map[midkey] = subnode2;
}

/* 0 = success */
/* 1 = element too full */
private int sub_set_element(mixed *node, mixed key, mixed value)
{
	int rebottom;

	if (node[0]) {
		mapping map;

		map = node[1];

		if (map[key] == nil && value != nil) {
			/* adding new element, check capacity */
			if (map_sizeof(map) == MAX_LEAF_SIZE) {
				return 1;
			}
		}

		map[key] = value;

		return 0;
	} else {
		int i;
		mixed *keys;
		int ret;
		mixed subkey;
		mixed *subnode;
		mapping map;

		keys = node[1];

		i = binary_search_floor(keys, key);

		if (i == -1) {
			i = 0;
			rebottom = 1;
		}

		subkey = keys[i];
		map = node[2];
		subnode = map[subkey];

		ret = sub_set_element(subnode, key, value);

		if (ret == 1) {
			/* subnode too big */
			if (sizeof(keys) >= MAX_BRANCH_SIZE) {
				/* we need to split but we're too big ourselves */
				return 1;
			}

			split_node(node, i);

			keys = node[1];

			if (keys[i + 1] <= key) {
				/* the split scooped our target */
				i++;
			}

			subkey = keys[i];
			subnode = node[2][subkey];

			ret = sub_set_element(subnode, key, value);
			ASSERT(ret != 1);
		}

		/* we can only rebottom if we're already on the ground */

		if (rebottom) {
			keys[0] = key;
			map[key] = map[subkey];
			map[subkey] = nil;
		} else {
			return 0;
		}
	}
}

void set_element(mixed key, mixed value)
{
	int ret;

	if (typeof(key) == 0 || typeof(key) != type) {
		error("Type mismatch");
	}

	if (ret = sub_set_element(root, key, value) == 1) {
		if (root[0]) {
			mixed basekey;

			basekey = map_indices(root[1])[0];
			root = ({ 0, ({ basekey }), ([ basekey : root ]) });
		} else {
			mixed basekey;

			basekey = root[1][0];
			root = ({ 0, ({ basekey }), ([ basekey : root ]) });
		}

		ret = sub_set_element(root, key, value);
	}
}

mixed query_element(mixed key)
{
	mixed *node;

	if (typeof(key) == 0 || typeof(key) != type) {
		error("Type mismatch");
	}

	node = root;

	while (!node[0]) {
		int i;
		mixed subkey;

		i = binary_search_floor(node[1], key);

		if (i == -1) {
			return nil;
		}

		subkey = node[1][i];
		node = node[2][subkey];
	}

	return node[1][key];
}

mixed *query_root()
{
	return root;
}

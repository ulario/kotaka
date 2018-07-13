#include <type.h>
#include <kotaka/paths/string.h>
#include <kotaka/assert.h>
#include <kotaka/paths/system.h>
#include <kotaka/log.h>

#define MAX_BRANCH_SIZE 4
#define MAX_LEAF_SIZE   4

inherit "/lib/search";

mapping root; /* ({ is_leaf, ({ keys }), ([ key : value/submap ]) }) */
int type;
int levels;

void set_type(int new_type)
{
	switch(new_type)
	{
	case T_INT:
	case T_FLOAT:
	case T_STRING:
		type = new_type;
		root = ([ ]);
		levels = 1;
		return;

	default:
		error("Invalid type");
	}
}

private void split_map(mapping map, int i)
{
	mixed *keys;
	mixed key;

	mixed *subkeys;
	mapping submap;

	int subsz;

	mixed midkey;

	mapping lowmap;
	mapping highmap;

	keys = map_indices(map);
	key = keys[i];

	submap = map[key];
	subkeys = map_indices(submap);

	subsz = sizeof(subkeys);
	subsz /= 2;

	midkey = subkeys[subsz];

	lowmap = submap[.. midkey];
	highmap = submap[midkey ..];
	lowmap[midkey] = nil;

	map[key] = lowmap;
	map[midkey] = highmap;
}

/* 0 = success */
/* 1 = element too full */
private int sub_set_element(mapping map, int level, mixed key, mixed value)
{
	int rebottom;

	if (!level) {
		if (map[key] == nil && value != nil) {
			/* adding new element, check capacity */
			if (map_sizeof(map) >= MAX_LEAF_SIZE) {
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
		mapping submap;

		keys = map_indices(map);

		i = binary_search_floor(keys, key);

		if (i == -1) {
			i = 0;
			rebottom = 1;
		}

		subkey = keys[i];
		submap = map[subkey];

		ret = sub_set_element(submap, level - 1, key, value);

		if (ret == 1) {
			/* submap too big */
			if (sizeof(keys) >= MAX_BRANCH_SIZE) {
				/* we need to split but we're too big ourselves */
				return 1;
			}

			split_map(map, i);

			keys = map_indices(map);

			if (keys[i + 1] <= key) {
				/* the split scooped our target */
				i++;
			}

			subkey = keys[i];
			submap = map[subkey];

			ret = sub_set_element(submap, level - 1, key, value);
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

	if (ret = sub_set_element(root, levels - 1, key, value) == 1) {
		mixed basekey;

		basekey = map_indices(root)[0];
		root = ([ basekey : root ]);
		levels++;

		ret = sub_set_element(root, levels - 1, key, value);
	}
}

mixed query_element(mixed key)
{
	mapping map;
	int level;

	if (typeof(key) == 0 || typeof(key) != type) {
		error("Type mismatch");
	}

	map = root;

	level = levels - 1;

	while (level) {
		int i;
		mixed *keys;

		keys = map_indices(map);

		i = binary_search_floor(keys, key);

		if (i == -1) {
			return nil;
		}

		map = map[keys[i]];
		level--;
	}

	return map[key];
}

mapping query_root()
{
	return root;
}

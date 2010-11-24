#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

inherit SECOND_AUTO;
inherit "../base/node";

mapping map;
int level;

static void create()
{
	::create();
	
	level = -1;
	map = ([ ]);
}

static void destruct()
{
	if (level) {
		object *turkeys;
		int sz;
		int i;
		
		turkeys = map_values(map);
		
		sz = sizeof(turkeys);
		for (i = 0; i < sz; i++) {
			discard_node(turkeys[i]);
		}
	}
}

void set_level(int new_level)
{
	check_caller();
	
	ASSERT(level == -1);
	
	level = new_level;
}

int get_level()
{
	check_caller();
	
	return level;
}

mapping get_map()
{
	check_caller();
	
	return map;
}

void set_map(mapping new_map)
{
	check_caller();
	
	map = new_map;
}

void dump()
{
	ACCESS_CHECK(SYSTEM());
	
	LOGD->post_message("dump", LOG_INFO, "Level " + level + " node.");
	
	if (level > 1) {
		int *indices;
		object *subnodes;
		int sz;
		int i;
		LOGD->post_message("dump", LOG_INFO, "({");
		
		indices = map_indices(map);
		subnodes = map_values(map);
		
		sz = sizeof(indices);
		LOGD->post_message("dump", LOG_INFO, sz + " subnodes");

		for (i = 0; i < sz; i++) {
			LOGD->post_message("dump", LOG_INFO, "index: " + indices[i]);
			subnodes[i]->dump();
		}
		
		LOGD->post_message("dump", LOG_INFO, "})");
	}
}

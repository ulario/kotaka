#include <kernel/access.h>

#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/bigstruct.h>

inherit SECOND_AUTO;
inherit "../base/root";

object *top;
int size;

atomic static void create()
{
	::create();
	
	top = allocate(4);
	top[0] = new_node();
	top[0]->set_level(0);
	size = 0;
}

static void destruct()
{
	int i;
	
	for (i = 0; i < 4; i++) {
		if (top[i]) {
			discard_node(top[i]);
		}
	}
}

private int mask(int index, int level)
{
	int bits;
	bits = level << 3;
	return (index >> bits) << bits;
}

private mixed sub_get_element(object node, int index)
{
	int level;
	int masked;
	mapping map;
	object subnode;
	
	level = node->get_level();
	map = node->get_map();
	
	if (level) {
		masked = mask(index, level);
		
		subnode = map[masked];
		
		if (subnode) {
			return sub_get_element(subnode, index);
		} else {
			return nil;
		}
	} else {
		return map[index];
	}
}

private void sub_set_element(object node, int index, mixed value)
{
	int level;
	int masked;
	mapping map;
	object subnode;
	
	level = node->get_level();
	map = node->get_map();
	
	if (level) {
		masked = mask(index, level);
		
		subnode = map[masked];
		
		if (subnode) {
			sub_set_element(subnode, index, value);
			
			if (map_sizeof(subnode->get_map()) == 0) {
				map[masked] = nil;
				discard_node(subnode);
			}
		} else {
			if (value != nil) {
				subnode = new_node();
				subnode->set_level(level - 1);
				
				map[masked] = subnode;
				sub_set_element(subnode, index, value);
			}
		}
	} else {
		map[index] = value;
	}
}

private void truncate_to(object node, int new_size)
{
	int level;
	mapping map;
	
	level = node->get_level();
	map = node->get_map();
	
	if (level) {
		/* elevated level */
		/* we need to properly dispose of any inferior nodes */
		int masked;
		int *indices;
		object *nodes;
		mapping tmap;
		int sz;
		
		masked = mask(new_size, level);
		tmap = map[masked ..];
		
		indices = map_indices(tmap);
		nodes = map_values(tmap);
		sz = sizeof(indices);
		
		if (sz) {
			int i;
			
			i = 0;
			
			if (indices[0] == masked) {
				/* filled node needs sub-level truncation */
				i++;
				
				truncate_to(nodes[0], new_size);

				if (!map_sizeof(nodes[0]->get_map())) {
					discard_node(nodes[0]);
					map[masked] = nil;
				}
			}
			
			for (; i < sz; i++) {
				discard_node(nodes[i]);
				map[indices[i]] = nil;
			}
		}
	} else {
		/* bottom level, just erase */
		map = map[.. new_size - 1];
		node->set_map(map);
	}
}

int get_size()
{
	check_caller(READ_ACCESS);

	return size;
}

int query_size()
{
	check_caller(READ_ACCESS);

	return size;
}

atomic void set_size(int new_size)
{
	int cur_level;
	int new_level;
	object node;
	
	check_caller(WRITE_ACCESS);
	
	if (new_size < 0) {
		error("Invalid argument");
	}
	
	if (size == new_size) {
		return;
	}
	
	switch(size)
	{
	case 0x00000000 .. 0x000000FF: cur_level = 0; break;
	case 0x00000100 .. 0x0000FFFF: cur_level = 1; break;
	case 0x00010000 .. 0x00FFFFFF: cur_level = 2; break;
	case 0x01000000 .. 0x7FFFFFFF: cur_level = 3; break;
	}
	
	switch(new_size)
	{
	case 0x00000000 .. 0x000000FF: new_level = 0; break;
	case 0x00000100 .. 0x0000FFFF: new_level = 1; break;
	case 0x00010000 .. 0x00FFFFFF: new_level = 2; break;
	case 0x01000000 .. 0x7FFFFFFF: new_level = 3; break;
	}
	
	if (new_size > size) {
		while (cur_level < new_level) {
			top[++cur_level] = new_node();
			top[cur_level]->set_level(cur_level);
		}
	}
	
	if (new_size < size) {
		while (cur_level > new_level) {
			discard_node(top[cur_level--]);
		}
		
		truncate_to(top[cur_level], new_size);
	}

	size = new_size;
}

atomic mixed get_element(int index)
{
	int level;
	
	check_caller(READ_ACCESS);
	
	if (index < 0) {
		error("Subscript out of range: " + index + " vs " + 0);
	}

	if (index >= size) {
		error("Subscript out of range: " + index + " vs " + size);
	}
	
	switch(index)
	{
	case 0x00000000 .. 0x000000FF: level = 0; break;
	case 0x00000100 .. 0x0000FFFF: level = 1; break;
	case 0x00010000 .. 0x00FFFFFF: level = 2; break;
	case 0x01000000 .. 0x7FFFFFFF: level = 3; break;
	}
	
	return sub_get_element(top[level], index);
}

atomic void set_element(int index, mixed value)
{
	int level;
	
	check_caller(WRITE_ACCESS);
	
	if (index < 0) {
		error("Subscript out of range");
	}

	if (index >= size) {
		error("Subscript out of range");
	}

	switch(index)
	{
	case 0x00000000 .. 0x000000FF: level = 0; break;
	case 0x00000100 .. 0x0000FFFF: level = 1; break;
	case 0x00010000 .. 0x00FFFFFF: level = 2; break;
	case 0x01000000 .. 0x7FFFFFFF: level = 3; break;
	}
	
	sub_set_element(top[level], index, value);
}

atomic void clear()
{
	int old_size;
	
	check_caller(WRITE_ACCESS);
	
	old_size = size;
	
	set_size(0);
	set_size(old_size);
}

object slice(int lowindex, int highindex)
{
	int count;
	int i;
	object slice;
	
	check_caller(READ_ACCESS);
	
	if (lowindex < 0 || highindex >= size) {
		error("Subscript out of range");
	}
	
	slice = new_object(BIGSTRUCT_ARRAY_LWO);
	slice->set_size(highindex - lowindex + 1);
	
	for (i = lowindex; i <= highindex; i++) {
		slice->set_element(
			i - lowindex, get_element(i)
		);
	}
	
	return slice;
}

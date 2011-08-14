#include <kernel/access.h>

#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <kotaka/bigstruct.h>

#include <type.h>

inherit SECOND_AUTO;
inherit "../bintree/root";

#define MIN_MASS 256
#define MAX_MASS 768

int type;

atomic static void create()
{
	::create();

	top = insert_node(nil);
}

static void destruct()
{
	::destruct();
}

void clear()
{
	check_caller(WRITE_ACCESS);

	::clear();

	top = insert_node(nil);
}

void set_type(int new_type)
{
	check_caller(WRITE_ACCESS);

	ASSERT(type == 0);

	switch (new_type) {
	case T_INT:
	case T_FLOAT:
	case T_STRING:
		break;
	default:
		error("Invalid type for bigstruct map key");
	}
	
	type = new_type;
}

private object find_node(mixed key)
{
	object node;
	node = top;

	while (node) {
		mixed this_key;

		this_key = node->get_low_key();

		if (this_key == nil) {
			/* empty node */
			return node;
		}

		if (key < this_key) {
			object new_node;

			new_node = node->get_left();

			if (new_node) {
				node = new_node;
			} else {
				/* reached end */
				return node;
			}
		} else {
			object next;

			next = next_node(node);

			if (!next) {
				/* reached end */
				return node;
			}

			if (key < next->get_low_key()) {
				/* bracketed */
				return node;
			} else {
				node = node->get_right();
			}
		}
	}
}

private void merge_node_left(object node)
{
	object prev;
	mapping map;

	prev = prev_node(node);

	map = node->get_map() + prev->get_map();

	node->set_map(map);
	node->set_size(map_sizeof(map));

	node->reset_low_key();
	
	prev->set_map( ([ ]) );
	delete_node(prev);
}

private void merge_node_right(object node)
{
	object next;
	mapping map;

	next = next_node(node);

	map = node->get_map() + next->get_map();

	node->set_map(map);
	node->set_size(map_sizeof(map));

	node->reset_low_key();

	next->set_map( ([ ]) );
	delete_node(next);
}

private void split_node_right(object node)
{
	object next;
	mixed *keys;
	int sz;

	mapping map;
	mapping low_map;
	mapping high_map;

	mixed low_key;
	mixed high_key;

	next = insert_node(next_node(node));

	map = node->get_map();
	keys = map_indices(map);

	sz = sizeof(keys);

	low_key = keys[sz / 2 - 1];
	high_key = keys[sz / 2];

	low_map = map[.. low_key];
	high_map = map[high_key ..];

	node->set_map(low_map);
	node->set_size(sz / 2);
	next->set_map(high_map);
	next->set_size((sz + 1) / 2);

	node->set_low_key(keys[0]);
	next->set_low_key(high_key);
}

private int mass_check(object node);

private void check_node(object node, varargs int back)
{
	int check;

	while (check = mass_check(node)) {
		if (check == -1) {
			if (back) {
				if (prev_node(node)) {
					merge_node_left(node);
				} else if (next_node(node)) {
					merge_node_right(node);
				} else {
					return;
				}
			} else {
				if (next_node(node)) {
					merge_node_right(node);
				} else if (prev_node(node)) {
					merge_node_left(node);
				} else {
					return;
				}
			}
		} else if (check == 1) {
			split_node_right(node);
		}
	}
}

private int mass_check(object node)
{
	int size;

	size = map_sizeof(node->get_map());

	node->set_size(size);

	if (size < MIN_MASS) {
		return -1;
	}

	if (size > MAX_MASS) {
		return 1;
	}
}

atomic void set_element(mixed key, mixed value)
{
	object node;
	mapping map;
	mixed lkey;
	int size;
	int change;

	check_caller(WRITE_ACCESS);

	if (typeof(key) != type) {
		error("Type mismatch");
	}

	node = find_node(key);
	map = node->get_map();

	if (map[key] == nil) {
		if (value == nil) {
			return; /* do nothing */
		} else {
			size = node->get_size() + 1;
			change = 1;
		}
	} else if (value == nil) {
		size = node->get_size() - 1;
		change = 1;
	} else {
		size = node->get_size();
	}

	map[key] = value;

	if (size < MAX_MASS || size > MAX_MASS) {
		check_node(node);
	} else if (change) {
		node->set_size(size);
	}
}

mixed get_element(mixed key)
{
	object node;

	check_caller(READ_ACCESS);

	if (typeof(key) != type) {
		error("Type mismatch");
	}

	node = find_node(key);
	ASSERT(node);

	return node->get_map()[key];
}

/* slicing */
object slice(mixed first, mixed last)
{
	check_caller(READ_ACCESS);
	
	ASSERT(typeof(first) == type || first == nil);
	ASSERT(typeof(last) == type || last == nil);
	error("Function not implemented");
}

object get_indices()
{
	object array;
	object node;
	mixed *indices;
	int index;

	check_caller(READ_ACCESS);

	array = new_object("~/lwo/bigstruct/array/root");

	node = leftest(top);

	while (node) {
		mapping map;
		int index2;
		int sz;

		map = node->get_map();
		indices = ::map_indices(map);
		sz = sizeof(indices);

		array->set_size(index + sz);

		for (index2 = 0; index2 < sz; index2++) {
			array->set_element(index + index2, indices[index2]);
		}

		index += sz;
		node = next_node(node);
	}

	return array;
}

object get_values()
{
	object array;
	object node;
	mixed *values;
	int index;
	
	check_caller(READ_ACCESS);
	
	array = new_object("~/lwo/bigstruct/array/root");
	
	node = leftest(top);
	
	while (node) {
		mapping map;
		int index2;
		int sz;
		
		map = node->get_map();
		values = ::map_values(map);
		sz = sizeof(values);
		
		array->set_size(index + sz);
		
		for (index2 = 0; index2 < sz; index2++) {
			array->set_element(index + index2, values[index2]);
		}
		
		index += sz;
		node = next_node(node);
	}
	
	return array;
}

void rebalance()
{
	check_caller(READ_ACCESS);
	
	::rebalance();
}

atomic void reindex()
{
	object node;
	object deque;
	int quota;
	int nodes;
	mapping map;

	/* require write access because this is an intense operation */
	check_caller(WRITE_ACCESS);

	deque = new_object(BIGSTRUCT_DEQUE_LWO);

	node = leftest(top);

	while (node != nil) {
		mixed *indices, *values;
		int sz, i;

		map = node->get_map();
		node = next_node(node);

		sz = map_sizeof(map);

		indices = map_indices(map);
		values = map_values(map);

		for (i = 0; i < sz; i++) {
			deque->push_back(indices[i]);
			deque->push_back(values[i]);
		}
	}

	nodes = 0;

	discard_node(top);
	top = nil;
	top = insert_node(nil);
	node = top;

	quota = (MIN_MASS + MAX_MASS) / 2;
	map = ([ ]);
	node->set_map(map);

	while (!deque->empty()) {
		mixed index;
		mixed value;

		if (quota == 0) {
			node->reset_low_key();
			map = ([ ]);
			node = insert_node(nil);
			node->set_map(map);
			quota = (MIN_MASS + MAX_MASS) / 2;
		}

		index = deque->get_front();
		deque->pop_front();
		value = deque->get_front();
		deque->pop_front();

		map[index] = value;

		quota--;
	}

	node->reset_low_key();
	::rebalance();
}

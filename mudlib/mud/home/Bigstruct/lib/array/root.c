/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014  Raymond Jennings
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
#include <kernel/access.h>
#include <kotaka/paths/bigstruct.h>

inherit "../base/root";

object *top;
int size;

atomic static void create()
{
	::create();

	top = allocate(4);
	size = 0;
}

private void purge_node(object node)
{
	if (node->query_level()) {
		object *turkeys;
		int sz;
		int i;

		turkeys = node->query_array() - ({ nil });

		sz = sizeof(turkeys);

		for (i = 0; i < sz; i++) {
			purge_node(turkeys[i]);
		}
	}

	discard_node(node);
}

static void destruct()
{
	int i;

	for (i = 0; i < 4; i++) {
		if (top[i]) {
			purge_node(top[i]);
		}
	}
}

private int mask(int index, int level)
{
	int bits;
	bits = level << 3;
	return (index >> bits) & 0xFF;
}

private mixed sub_query_element(object node, int index)
{
	int level;
	int masked;
	mixed *array;
	object subnode;

	level = node->query_level();
	array = node->query_array();
	masked = mask(index, level);

	if (level) {
		subnode = array[masked];

		if (subnode) {
			return sub_query_element(subnode, index);
		} else {
			return nil;
		}
	} else {
		return array[masked];
	}
}

private void sub_set_element(object node, int index, mixed value)
{
	int level;
	int masked;
	mixed *array;
	object subnode;

	level = node->query_level();
	array = node->query_array();
	masked = mask(index, level);

	if (level) {
		subnode = array[masked];

		if (subnode) {
			sub_set_element(subnode, index, value);
		} else {
			if (value != nil) {
				subnode = new_node();
				subnode->set_level(level - 1);

				array[masked] = subnode;
				sub_set_element(subnode, index, value);
			}
		}
	} else {
		array[masked] = value;
	}
}

private void truncate_to(object node, int new_size)
{
	int level;
	int masked;
	int oldmasked;
	mixed *array;

	level = node->query_level();
	array = node->query_array();
	masked = mask(new_size - 1, level);
	oldmasked = mask(size - 1, level);

	if (level) {
		int i;

		for (i = masked + 1; i <= oldmasked; i++) {
			if (array[i]) {
				purge_node(array[i]);
				array[i] = nil;
			}
		}

		if (array[masked]) {
			truncate_to(array[masked], new_size);
		}
	} else {
		int i;

		for (i = masked + 1; i <= oldmasked; i++) {
			array[i] = nil;
		}
	}
}

int empty()
{
	check_caller(READ_ACCESS);

	return !size;
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
	case 0x00000000: cur_level = -1; break;
	case 0x00000001 .. 0x00000100: cur_level = 0; break;
	case 0x00000101 .. 0x00010000: cur_level = 1; break;
	case 0x00010001 .. 0x01000000: cur_level = 2; break;
	case 0x01000001 .. 0x7FFFFFFF: cur_level = 3; break;
	}

	switch(new_size)
	{
	case 0x00000000: new_level = -1; break;
	case 0x00000001 .. 0x00000100: new_level = 0; break;
	case 0x00000101 .. 0x00010000: new_level = 1; break;
	case 0x00010001 .. 0x01000000: new_level = 2; break;
	case 0x01000001 .. 0x7FFFFFFF: new_level = 3; break;
	}

	if (new_size > size) {
		while (cur_level < new_level) {
			object node;

			++cur_level;
			node = new_node();
			node->set_level(cur_level);
			top[cur_level] = node;
		}
	}

	if (new_size < size) {
		while (cur_level > new_level) {
			purge_node(top[cur_level]);
			top[cur_level--] = nil;
		}

		if (cur_level >= 0) {
			truncate_to(top[cur_level], new_size);
		}
	}

	size = new_size;
}

atomic mixed query_element(int index)
{
	int level;

	check_caller(READ_ACCESS);

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

	return sub_query_element(top[level], index);
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

atomic void push_back(mixed data)
{
	int sz;

	check_caller(WRITE_ACCESS);

	set_size(size + 1);
	set_element(size - 1, data);
}

atomic void pop_back()
{
	check_caller(WRITE_ACCESS);

	set_size(size - 1);
}

mixed query_front()
{
	check_caller(READ_ACCESS);

	return query_element(0);
}

mixed query_back()
{
	check_caller(READ_ACCESS);

	return query_element(size - 1);
}

void set_front(mixed value)
{
	check_caller(WRITE_ACCESS);

	set_element(0, value);
}

void set_back(mixed value)
{
	check_caller(WRITE_ACCESS);

	set_element(size - 1, value);
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

	/* todo:  use array operations to handle this task in bulk */

	check_caller(READ_ACCESS);

	if (lowindex < 0 || highindex >= size) {
		error("Subscript out of range");
	}

	slice = new_object(BIGSTRUCT_ARRAY_LWO);
	slice->set_size(highindex - lowindex + 1);

	for (i = lowindex; i <= highindex; i++) {
		slice->set_element(
			i - lowindex, query_element(i)
		);
	}

	slice->grant_access(previous_object(), FULL_ACCESS);
	slice->grant_access(this_object(), 0);

	return slice;
}

/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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

inherit "../base/root";

object first;
object last;

void dump();

private void trim_first()
{
	if (first->get_mass() == 0 && first != last) {
		object old;

		old = first;
		first = first->get_next();
		old->set_next(nil);

		discard_node(old);
	}
}

private void trim_last()
{
	if (last->get_mass() == 0 && first != last) {
		object old;

		old = last;
		last = last->get_prev();

		discard_node(old);
	}
}

atomic static void create()
{
	::create();

	first = last = new_node();
}

static void destruct()
{
	while (first) {
		object turkey;
		turkey = first;
		first = first->get_next();
		discard_node(turkey);
	}
}

mixed get_front()
{
	check_caller(READ_ACCESS);

	return first->get_front();
}

mixed get_back()
{
	check_caller(READ_ACCESS);

	return last->get_back();
}

void set_front(mixed value)
{
	check_caller(WRITE_ACCESS);

	first->set_front(value);
}

void set_back(mixed value)
{
	check_caller(WRITE_ACCESS);

	last->set_back(value);
}

atomic void push_front(mixed value)
{
	check_caller(WRITE_ACCESS);

	if (first->full_front()) {
		object new;

		new = new_node();

		first->set_prev(new);
		new->set_next(first);
		first = new;
	}

	trim_last();
	first->push_front(value);
}

atomic void push_back(mixed value)
{
	check_caller(WRITE_ACCESS);

	if (last->full_back()) {
		object new;

		new = new_node();

		last->set_next(new);
		new->set_prev(last);
		last = new;
	}

	last->push_back(value);
	trim_first();
}

atomic void pop_front()
{
	check_caller(WRITE_ACCESS);

	first->pop_front();
	trim_first();
}

atomic void pop_back()
{
	check_caller(WRITE_ACCESS);

	last->pop_back();
	trim_last();
}

int empty()
{
	check_caller(READ_ACCESS);

	return first->empty();
}

atomic void clear()
{
	check_caller(WRITE_ACCESS);

	if (first) {
		object turkey;

		turkey = first;
		first = last = new_node();

		discard_node(turkey);
	}
}

int get_size()
{
	object node;
	int mass;

	check_caller(READ_ACCESS);

	mass = 0;

	node = first;

	while (node) {
		mass += node->get_mass();
		node = node->get_next();
	}

	return mass;
}

mixed get_element(int index)
{
	object node;
	int mass;

	check_caller(READ_ACCESS);

	node = first;

	while (node && (mass = node->get_mass()) > index) {
		index -= mass;
	}

	if (!node) {
		error("Subscript out of range");
	}

	return node->get_element(index);
}

void set_element(int index, mixed value)
{
	object node;
	int mass;

	check_caller(WRITE_ACCESS);

	node = first;

	while (node && (mass = node->get_mass()) > index) {
		index -= mass;
	}

	if (!node) {
		error("Subscript out of range");
	}

	node->set_element(index, value);
}

void dump()
{
	object node;

	ACCESS_CHECK(SYSTEM());

	node = first;

	while (node) {
		LOGD->post_message("dump", LOG_DEBUG, "Dumping node " + object_name(node));
		node->dump();
		node = node->get_next();
	}
}

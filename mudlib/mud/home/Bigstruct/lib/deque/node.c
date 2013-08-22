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
#include <kotaka/assert.h>
#include <status.h>

inherit "../base/node";

mixed *sub;
object prev;
object next;

int begin;
int end;

static void create()
{
	int arrsz;

	::create();

	arrsz = status(ST_ARRAYSIZE);

	sub = allocate(arrsz);
	begin = end = arrsz / 2;
}

object get_prev()
{
	check_caller();
	return prev;
}

object get_next()
{
	check_caller();
	return next;
}

int get_mass()
{
	check_caller();
	return end - begin;
}

int empty()
{
	check_caller();
	return begin == end;
}

int get_begin()
{
	check_caller();
	return begin;
}

int get_end()
{
	check_caller();
	return end;
}

int full_front()
{
	check_caller();
	return begin == 0;
}

int full_back()
{
	check_caller();
	return end == sizeof(sub);
}

void set_prev(object new)
{
	check_caller();
	prev = new;
}

void set_next(object new)
{
	check_caller();
	next = new;
}

mixed get_front()
{
	check_caller();

	if (begin >= end) {
		error("Subscript out of range");
	}

	return sub[begin];
}

mixed get_back()
{
	check_caller();

	if (begin >= end) {
		error("Subscript out of range");
	}

	return sub[end - 1];
}

void set_front(mixed value)
{
	check_caller();
	ASSERT(begin < end);

	sub[begin] = value;
}

void set_back(mixed value)
{
	check_caller();
	ASSERT(begin < end);

	sub[end - 1] = value;
}

void push_front(mixed value)
{
	check_caller();

	ASSERT(begin > 0);

	sub[--begin] = value;
}

void push_back(mixed value)
{
	check_caller();

	ASSERT(end < sizeof(sub));

	sub[end++] = value;
}

void pop_front()
{
	check_caller();
	ASSERT(begin < end);

	sub[begin++] = nil;
}

void pop_back()
{
	check_caller();
	ASSERT(begin < end);

	sub[--end] = nil;
}

mixed query_element(int index)
{
	check_caller();

	if (index < begin || index >= end) {
		error("Subscript out of range");
	}

	return sub[index];
}

void set_element(int index, mixed data)
{
	check_caller();

	if (index < begin || index >= end) {
		error("Subscript out of range");
	}

	sub[index] = data;
}

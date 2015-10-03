/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2010, 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/privilege.h>

inherit "../base/node";

private object parent;
private object left;
private object right;

static void create()
{
	::create();
}

static void destruct()
{
	::destruct();
}

object query_parent()
{
	ACCESS_CHECK(BIGSTRUCT());

	return parent;
}

object query_left()
{
	ACCESS_CHECK(BIGSTRUCT());

	return left;
}

object query_right()
{
	ACCESS_CHECK(BIGSTRUCT());

	return right;
}

void set_parent(object new_parent)
{
	ACCESS_CHECK(BIGSTRUCT());

	parent = new_parent;
}

void set_left(object new_left)
{
	ACCESS_CHECK(BIGSTRUCT());

	left = new_left;
}

void set_right(object new_right)
{
	ACCESS_CHECK(BIGSTRUCT());

	right = new_right;
}

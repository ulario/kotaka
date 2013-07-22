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
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <status.h>

inherit "../base/node";

private object parent;
private object left;
private object right;

static void create()
{
}

static void destruct()
{
}

object get_parent()
{
	check_caller();
	return parent;
}

object get_left()
{
	check_caller();
	return left;
}

object get_right()
{
	check_caller();
	return right;
}

void set_parent(object new)
{
	check_caller();
	parent = new;
}

void set_left(object new)
{
	check_caller();
	left = new;
}

void set_right(object new)
{
	check_caller();
	right = new;
}

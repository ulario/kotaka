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

#include <kotaka/assert.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

private mapping grants;
private int global_access;

static object new_node();
static void discard_node(object node);

static void create()
{
	grants = ([ previous_object() : FULL_ACCESS ]);
}

static void destruct()
{
}

private int access_of(object obj)
{
	int highest;

	if (grants[obj]) {
		highest = grants[obj];
	}

	if (highest > global_access) {
		return highest;
	} else {
		return global_access;
	}
}

static void check_caller(int access)
{
	object pobj;

	pobj = previous_object();

	if (!sscanf(object_name(pobj),
		USR_DIR + "/Bigstruct/%*s/"))
	{
		ACCESS_CHECK(access_of(pobj) >= access);
	}
}

mapping query_grants()
{
	check_caller(READ_ACCESS);

	return grants[..];
}

int query_global_access()
{
	check_caller(READ_ACCESS);

	return global_access;
}

void grant_access(object obj, int access)
{
	check_caller(FULL_ACCESS);

	grants[obj] = access ? access : nil;
}

void grant_global_access(int access)
{
	check_caller(FULL_ACCESS);

	global_access = access;
}

void self_destruct()
{
	ACCESS_CHECK(BIGSTRUCT());

	destruct_object(this_object());
}

/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <kotaka/assert.h>
#include <status.h>

inherit SECOND_AUTO;

string path;			/* the canonical path for this object */

int *inherits;			/* indices of inherited objects */
string *includes;		/* canonical include files */

int destructed;

string constructor;
string *inherited_constructors;

string destructor;
string *inherited_destructors;

string patcher;
string *inherited_patchers;

int nclones;
mapping clones;

static void create(varargs int clone)
{
	if (clone) {
		clones = ([ ]);
	}
}

string query_path()
{
	return path;
}

int *query_inherits()
{
	return inherits ? inherits[..] : nil;
}

string *query_includes()
{
	return includes ? includes[..] : nil;
}

int query_destructed()
{
	return destructed;
}

string query_constructor()
{
	return constructor;
}

string query_destructor()
{
	return destructor;
}

string query_patcher()
{
	return patcher;
}

string *query_inherited_constructors()
{
	return inherited_constructors ? inherited_constructors[..] : ({ });
}

string *query_inherited_destructors()
{
	return inherited_destructors ? inherited_destructors[..] : ({ });
}

string *query_inherited_patchers()
{
	return inherited_patchers ? inherited_patchers[..] : ({ });
}

int query_clone_count()
{
	return nclones;
}

object *query_clones()
{
	ACCESS_CHECK(SYSTEM());

	return clones ? map_indices(clones) : nil;
}

void set_path(string new_path)
{
	ACCESS_CHECK(SYSTEM());

	path = new_path;
}

void set_inherits(int *new_inherits)
{
	ACCESS_CHECK(SYSTEM());

	inherits = new_inherits ? new_inherits[..] : nil;
}

void set_includes(string *new_includes)
{
	ACCESS_CHECK(SYSTEM());

	includes = new_includes ? new_includes[..] : nil;
}

void set_destructed()
{
	ACCESS_CHECK(SYSTEM());

	destructed = 1;
}

void set_constructor(string new_constructor)
{
	ACCESS_CHECK(SYSTEM());

	constructor = new_constructor;
}

void set_destructor(string new_destructor)
{
	ACCESS_CHECK(SYSTEM());

	destructor = new_destructor;
}

void set_patcher(string new_patcher)
{
	ACCESS_CHECK(SYSTEM());

	patcher = new_patcher;
}

void set_inherited_constructors(string *constructors)
{
	ACCESS_CHECK(SYSTEM());

	inherited_constructors = sizeof(constructors) ? constructors[..] : nil;
}

void set_inherited_destructors(string *destructors)
{
	ACCESS_CHECK(SYSTEM());

	inherited_destructors = sizeof(destructors) ? destructors[..] : nil;
}

void set_inherited_patchers(string *patchers)
{
	ACCESS_CHECK(SYSTEM());

	inherited_patchers = sizeof(patchers) ? patchers[..] : nil;
}

void clear_clones()
{
	ACCESS_CHECK(SYSTEM());

	ASSERT(path);
	ASSERT(sscanf(path, "%*s" + CLONABLE_SUBDIR + "%*s"));

	nclones = 0;
	clones = ([ ]);
}

void add_clone(object clone)
{
	ACCESS_CHECK(SYSTEM());

	nclones++;

	if (nclones >= status(ST_ARRAYSIZE)) {
		clones = nil;
	} else {
		clones[clone] = 1;
	}
}

void remove_clone(object clone)
{
	ACCESS_CHECK(SYSTEM());

	nclones--;

	if (clones) {
		clones[clone] = nil;
	}
}

atomic void reset_clones()
{
	int sz;

	ACCESS_CHECK(SYSTEM());

	ASSERT(path);
	ASSERT(sscanf(path, "%*s" + CLONABLE_SUBDIR + "%*s"));

	clear_clones();

	for (sz = status(ST_OTABSIZE); --sz >= 0; ) {
		object obj;

		if (obj = find_object(path + "#" + sz)) {
			add_clone(obj);
		}
	}
}

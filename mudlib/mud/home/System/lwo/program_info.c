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
#include <kernel/kernel.h>

#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <kotaka/assert.h>

#include <status.h>

inherit SECOND_AUTO;

/*************/
/* Variables */
/*************/

string path;			/* the canonical path for this object */

int *inherits;			/* program numbers of inherited objects */
string *includes;		/* canonical include files */

int destructed;			/* destructed */

string constructor;		/* constructor */
string destructor;		/* destructor */

string *inherited_constructors;	/* constructors */
string *inherited_destructors;	/* destructors */

object first;			/* first clone */

/****************/
/* Declarations */
/****************/

static void create(int clone);
void set_path(string new_path);
void set_inherits(int *new_inherits);
void set_includes(string *new_includes);
void set_destructed();

void set_constructor(string constructor);
void set_destructor(string destructor);

void set_inherited_constructors(string *constructors);
void set_inherited_destructors(string *destructors);

void set_first_clone(object obj);

string query_path();
int *query_inherits();
string *query_includes();
int query_destructed();

string query_constructor();
string query_destructor();

string *query_inherited_constructors();
string *query_inherited_destructors();

object query_first_clone();

/***************/
/* definitions */
/***************/

static void create(int clone)
{
}

void set_path(string new_path)
{
	ACCESS_CHECK(SYSTEM());

	path = new_path;
}

void set_inherits(int *new_inherits)
{
	ACCESS_CHECK(SYSTEM());

	inherits = new_inherits;
}

void set_includes(string *new_includes)
{
	ACCESS_CHECK(SYSTEM());

	includes = new_includes;
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

void set_inherited_constructors(string *constructors)
{
	ACCESS_CHECK(SYSTEM());

	inherited_constructors = constructors;
}

void set_inherited_destructors(string *destructors)
{
	ACCESS_CHECK(SYSTEM());

	inherited_destructors = destructors;
}

void set_first_clone(object obj)
{
	ACCESS_CHECK(SYSTEM());

	first = obj;
}

string query_path()
{
	return path;
}

int *query_inherits()
{
	return inherits[..];
}

string *query_includes()
{
	return includes[..];
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

string *query_inherited_constructors()
{
	return inherited_constructors;
}

string *query_inherited_destructors()
{
	return inherited_destructors;
}

object query_first_clone()
{
	return first;
}

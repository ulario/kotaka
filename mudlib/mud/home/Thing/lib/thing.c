/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012-2013  Raymond Jennings
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
#include <kotaka/paths.h>

inherit arch "archetype";
inherit id "id";
inherit inv "inventory";
inherit prop "property";
inherit bulk "bulk";
inherit exit "exit";

int destructing;

/*****************/
/* General stuff */
/*****************/

static void create()
{
	arch::create();
	id::create();
	inv::create();
	prop::create();
	bulk::create();
}

int forbid_insert(object obj)
{
	return destructing;
}

static void move_notify(object old_env)
{
	id::move_notify(old_env);
	bulk::move_notify(old_env);
}

static nomask void thing_destruct()
{
	int sz;
	int index;
	object env;
	object *children;

	destructing = 1;

	children = query_inventory();
	env = query_environment();
	sz = sizeof(children);

	for (index = 0; index < sz; index++) {
		children[index]->move(env);
	}

	if (env) {
		env->bulk_invalidate();
	}
}

void clean()
{
	clean_archetypes();
	clean_inventory();
	clean_properties();
}

void self_destruct()
{
	destruct_object(this_object());
}
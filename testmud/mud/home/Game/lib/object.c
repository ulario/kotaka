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
#include <kotaka/paths.h>
#include <game/paths.h>

inherit base LIB_OBJECT;

inherit position "position";
inherit bulk "bulk";

int destructing;

/*****************/
/* General stuff */
/*****************/

static void create()
{
	string name;
	string *parts;
	int sz;

	base::create();
	bulk::create();
	position::create();

	name = object_name(this_object());

	sscanf(name, "%s#%*d", name);

	parts = explode(name, "/");
	sz = sizeof(parts);

	set_property("id", parts[sz - 1]);
}

int forbid_insert(object obj)
{
	return destructing;
}

static void move_notify(object old_env)
{
	object env;

	if (env = query_environment()) {
		env->bulk_invalidate();
	}

	if (old_env) {
		old_env->bulk_invalidate();
	}
}

static nomask void game_object_destruct()
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
}

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
inherit arch "thing/archetype";
inherit id "thing/id";
inherit inv "thing/inventory";
inherit prop "thing/property";
inherit bulk "thing/bulk";

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

static void destruct()
{
	object env;

	env = query_environment();

	if (env) {
		env->bulk_invalidate();
	}
}

void self_destruct()
{
	destruct_object(this_object());
}

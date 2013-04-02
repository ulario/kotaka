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
#include <kotaka/checkarg.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

inherit "archetype";
inherit "inventory";
inherit "property";

private void initialize()
{
	initialize_archetype();
	initialize_inventory();
	initialize_property();
}

static void create()
{
	initialize();
}

nomask void thing_object_constructor()
{
	ACCESS_CHECK(previous_program() == SECOND_AUTO);

	initialize();
}

nomask void thing_object_destructor()
{
	ACCESS_CHECK(previous_program() == SECOND_AUTO);
}

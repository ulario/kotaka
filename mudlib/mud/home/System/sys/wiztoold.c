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

#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/checkarg.h>

inherit SECOND_AUTO;

mapping wiztools;

static void create()
{
	wiztools = ([ ]);
}

void reboot()
{
	int sz;
	object *wiz;

	ACCESS_CHECK(SYSTEM());

	wiz = map_values(wiztools);
	sz = sizeof(wiz);

	while (--sz >= 0) {
		destruct_object(wiz[sz]);
	}
}

object get_wiztool()
{
	string name;
	object wiztool;

	ACCESS_CHECK(PRIVILEGED());

	name = this_user()->query_name();

	if (!name) {
		error("Only users can have wiztools");
	}

	wiztool = wiztools[name];

	if (!wiztool) {
		wiztool = wiztools[name] = clone_object("~/obj/wiztool", name);
		wiztool->set_user(previous_object());
	}

	return wiztools[name];
}

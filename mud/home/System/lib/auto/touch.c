/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014, 2015  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <status.h>

inherit "call_guard";

static void call_touch(object obj)
{
	/* anyone can use call_touch */

	if (previous_program() == TOUCHD) {
		::call_touch(obj);
	} else {
		TOUCHD->touch_object(obj);
	}
}

static void touch()
{
}

nomask void _F_touch(string func)
{
	object this;
	string name;
	string *patches;
	int oindex;
	int sz;

	ACCESS_CHECK(previous_program() == OBJECTD);

	this = this_object();
	name = object_name(this);

	if (!sscanf(name, "%*s#%d", oindex)) {
		oindex = status(this, O_INDEX);
	}

	patches = PATCHD->query_patches(oindex);
	PATCHD->clear_patches(oindex);

	if (patches) {
		for (sz = sizeof(patches) - 1; sz >= 0; --sz) {
			catch {
				call_other(this_object(), patches[sz]);
			}
		}
	}

	touch();
}
